## Generate debug symbol ##
|APP-D|YES|
|:----|:--|
|APP-R|YES|
|LIB-D|YES|
|LIB-R|YES|
The debug symbol for xcode project is controlled by project settings of "Apple LLVD compiler 4.2"/"generate debug symbol". The recommendation is setting it to YES for both debug and release build. So that it can be used for crash report symbolization.
For application, this will generate a separate dsym file in build output folder. For static library, this will only increase the size of the generated a file

## Deployment postprocessing ##
|APP-D|NO|
|:----|:-|
|APP-R|YES|
|LIB-D|NO|
|LIB-R|NO|
This is a generic control to decide whether other deployment operation should be performed, particular, Strip Linked Product is executed or not.
For static library, the recommended setting is NO for both release and debug build, to not remove symbols.
For application build, the recommended setting is Yes for release build and No for debug build. So that release build can have a smaller size.

## Strip Debug Symbols During copy ##
|APP-D|NO|
|:----|:-|
|APP-R|NO|
|LIB-D|NO|
|LIB-R|NO|
This flag decides when copying the dependent library for building the project, should the library's symbol be removed. It does not affect the build output binary.
The recommended setting is set to NO for both release and debug build, as the build output needs to include the dependent libraries' symbol to debug or symbolize the library code.
(It seems for ios project, this setting does not have any effect.)

## Strip linked product ##
|APP-D|YES|
|:----|:--|
|APP-R|NO |
|LIB-D|NO |
|LIB-R|NO |
This flag will reduce the size of the executable by removing the symbol from it. But it will cause the crash dump not have any symbol, and it will need a separate symbol file to symbolize the crash report.
The flag will not change sym file size.
For library, the recommended setting is NO, so it includes the debug symbol in the .a file
For application, if the app size matters, set it to YES. otherwise, set it to NO.