#include "../include/dgtcontext.h"

#include <iostream>

using namespace std;

//tuple<vector<ViInt16>, vector<ViInt64>, vector<ViReal64>> DgtContext::acquire(const int32_t triggers, std::chrono::milliseconds timeoutMs)
//{
//	cout << "\nacquire" << endl;
//
//	ViStatus ec;
//	ViInt64 acquiredRecords = 0;
//	ViInt64 *acquiredPoints = new ViInt64[triggers];
//	std::vector<ViInt64> firstValidPoints(triggers);
//	
//	// ?
//	ViReal64 *initialXOffset = new ViReal64[triggers], *initialXTimeFraction = new ViReal64[triggers];
//	ViReal64 xIncrement = 0.0, scaleFactor = 0.0, scaleOffset = 0.0;
//	std::vector<ViReal64> initialXTimeSeconds(triggers);
//	// end ?
//
//	//AqMD3_SetAttributeViInt64(session, "", AQMD3_ATTR_NUM_RECORDS_TO_ACQUIRE, triggers);
//
//	AqMD3_InitiateAcquisition(session);
//
//	AqMD3_WaitForAcquisitionComplete(session, timeoutMs.count());
//
//	ViInt64 acquiredArraySize = 0;
//	AqMD3_QueryMinWaveformMemory(session, 16, triggers, 0, 94016, &acquiredArraySize);
//
//	std::vector<ViInt16> buffer(acquiredArraySize);
//	
//	AqMD3_FetchMultiRecordWaveformInt16(session, "Channel1", 0, triggers, 0, 94016, acquiredArraySize,
//		&buffer[0], &acquiredRecords, acquiredPoints, firstValidPoints.data(), initialXOffset,
//		initialXTimeSeconds.data(), initialXTimeFraction, &xIncrement, &scaleFactor, &scaleOffset);
//
//	delete acquiredPoints, firstValidPoints;
//	delete initialXOffset, initialXTimeSeconds, initialXTimeFraction;
//
//	return std::make_tuple(buffer, firstValidPoints, initialXTimeSeconds);
//}
