# LocalUI

This is a C# project building a execuatable file. This file will call LocalUIDll.dll built by the C++/CLI project LocalUIDll when executing. Unfortunately, to correctly call the dll, the property of this project should be compatible with that of LocalUIDll. The most important is that the default target platform of a C# project is MSIL (which is marked as "Any CPU") while the target platform of a C++/CLI project can only be x86 or AMD64 (although there is an "Any CPU" Option). Therefore, the target platform of the C# project must be set to x86 if that if C++/CLI is x86, the same for x64, or when executing, it will fail to load some run-time libraries. 

> 这是一个生成可执行文件的C#项目。这个文件在执行时会调用C++/CLI项目LocalUIDll生成的LocalUIDll.dll。不幸的是，要正确调用这个dll，项目的属性必须与LocalUIDll的项目属性兼容。最重要的是，C#项目的默认目标平台是MSIL（被标记为“Any CPU”），而C++/CLI的目标平台只能是x86或AMD64（尽管这里也有一个“Any CPU”选项）。因此，如果C++/CLI项目的目标平台是x86，C#项目的目标平台只能是x86，x64也同理，否则当执行的时候，程序会加载某些运行时库失败。



