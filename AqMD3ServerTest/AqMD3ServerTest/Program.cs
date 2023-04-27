using NetMQ.Sockets;
using NetMQ;
using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using IronSnappy;

namespace AqMD3ServerTest
{
    internal class Program
    {
        static void Main(string[] args)
        {
            var initTimeoutS = 10;
            var otherTimeoutS = 10;

            using (var client = new RequestSocket("tcp://localhost:5555"))
            {
                var r = false;
                var message = new NetMQMessage();
                message.Append("init");
                client.SendMultipartMessage(message);
                r = client.TryReceiveFrameString(TimeSpan.FromSeconds(initTimeoutS), out var rMsgInit);

                if (!r)
                {
                    initTimeoutS += 5;
                    return;
                }

                message = new NetMQMessage();
                message.Append("horizontal");
                message.Append("0.0000000005");
                client.SendMultipartMessage(message);
                r = client.TryReceiveFrameString(TimeSpan.FromSeconds(otherTimeoutS), out var rMsgHorizontal);

                if (!r)
                {
                    otherTimeoutS += 1;
                    return;
                }

                message = new NetMQMessage();
                message.Append("vertical");
                message.Append("0.255");
                client.SendMultipartMessage(message);
                r = client.TryReceiveFrameString(TimeSpan.FromSeconds(otherTimeoutS), out var rMsgVertical);

                if (!r)
                {
                    initTimeoutS += 1;
                    return;
                }

                message = new NetMQMessage();
                message.Append("invert");
                message.Append("false");
                client.SendMultipartMessage(message);
                r = client.TryReceiveFrameString(TimeSpan.FromSeconds(otherTimeoutS), out var rMsgInvert);

                if (!r)
                {
                    otherTimeoutS += 1;
                    return;
                }

                message = new NetMQMessage();
                message.Append("trig class");
                message.Append("trig source");
                message.Append("mode");
                message.Append("config digitizer");
                message.Append("post samples");
                message.Append("pre samples");
                message.Append("setup array");
                client.SendMultipartMessage(message);
                r = client.TryReceiveFrameString(TimeSpan.FromSeconds(initTimeoutS), out var rMsgBlock);

                if (!r)
                {
                    initTimeoutS += 1;
                }
            }

            Task.Run(
                () =>
                {
                    using (var sub = new SubscriberSocket())
                    {
                        sub.Subscribe("data");
                        sub.Connect("tcp://localhost:5554");
                        while (true)
                            try
                            {
                                var topic = sub.ReceiveFrameString(out var more);

                                var message = sub.ReceiveFrameBytes(out more);
                                var decompressedMessage = Snappy.Decode(message);
                                var dataMessage = Message.Parser.ParseFrom(decompressedMessage);
                                Console.WriteLine($"msg len {dataMessage.Mz.Count}");
                            }
                            catch (OperationCanceledException ex)
                            {
                            }
                            catch (Exception ex)
                            {
                                Console.WriteLine($"{ex}");
                            }
                    }
                });

            while (true)
            {
                TofWidthMessage tofWidthMessage = null;
                using (var reqClient = new RequestSocket("tcp://localhost:5555"))
                {
                    var message = new NetMQMessage();
                    message.Append("acquire");
                    if (reqClient.TrySendMultipartMessage(TimeSpan.FromMilliseconds(35000), message))
                        if (reqClient.TryReceiveFrameBytes(TimeSpan.FromMilliseconds(35000), out var width))
                        {
                            tofWidthMessage = TofWidthMessage.Parser.ParseFrom(width);
                            if (tofWidthMessage == null)
                            {
                                throw new Exception("TOF width cannot be null!");
                            }

                            if (reqClient.TryReceiveFrameString(TimeSpan.FromMilliseconds(3500), out var hash))
                            {
                                var hashing = System.Security.Cryptography.SHA256Cng.Create();
                                var hashedVal = hashing.ComputeHash(width);

                            }
                        }
                }

                Console.WriteLine($"{tofWidthMessage.NumSamples}");
                var key = Console.ReadKey().Key;

                using (var client = new RequestSocket("tcp://localhost:5555"))
                {
                    var r = false;
                    var message = new NetMQMessage();
                    message.Append("stop");
                    client.SendMultipartMessage(message);
                    r = client.TryReceiveFrameString(TimeSpan.FromSeconds(initTimeoutS), out var rMsgInit);
                }

                if (key == ConsoleKey.Tab) 
                {
                    break;
                }
            }
        }
    }
}
