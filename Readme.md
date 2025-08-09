# AqMD3 Acquisition Console

An application for controlling and acquiring data from an Acqiris SA220P digitizer card intended for use with Ion-Mobility Spectrometry systems.


## Building

Microsoft Visual Studio 2019 version 16.9 or newer required.

Third-party dependencies are managed using vcpkg, the utility and installation instructions can be found [here](https://github.com/microsoft/vcpkg).

Once vcpkg is installed, navigate to the directoy containing vcpkg.exe and execute the following command to install required third-party dependencies:
```powershell
PS C:\vcpkg> .\vcpkg.exe install zeromq:x64-windows sqlitecpp:x64-windows sqlite3:x64-windows snappy:x64-windows protobuf:x64-windows picosha2:x64-windows cppzmq:x64-windows
```

The **CMAKE_TOOLCHAIN_FILE** variable must have its value updated to be the file path for vcpkg.cmake, an example of this is shown [here](https://github.com/microsoft/vcpkg/blob/master/docs/examples/installing-and-using-packages.md) under **Section 2: Use**.

Acqiris MD3 Software and drivers for the SA220P must be installed on the system to both run and build the application. The software installer is made available by Acqiris and can be found [here](https://extranet.acqiris.com/homepage?field_res_products_target_id=23).



## Contacts

Written by Cameron Giberson and Joon-Yong Lee for the Department of Energy (PNNL, Richland, WA)\
Copyright 2021, Battelle Memorial Institute. All Rights Reserved.\
E-mail: cameron.giberson@pnnl.gov or proteomics@pnnl.gov\
Website: https://omics.pnl.gov/ or https://panomics.pnnl.gov/


## License

AqMD3 Acquisition Console is licensed under the 2-Clause BSD License; you may not use this program 
except in compliance with the License. You may obtain a copy of the License at 
https://opensource.org/licenses/BSD-2-Clause

Copyright 2021 Battelle Memorial Institute