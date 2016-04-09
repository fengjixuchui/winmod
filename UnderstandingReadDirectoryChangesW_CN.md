# 理解 ReadDirectoryChangesW #

  * 原作者：Jim Beveridge
  * 原文：http://qualapps.blogspot.com/2010/05/understanding-readdirectorychangesw.html?amp
  * 渣翻译：bbcallen@gmail.com

_世界上最长，最详细的 ReadDirectoryChangesW 的使用方法描述。_

[下载本文的示例代码](http://cid-e030bb50921fff08.skydrive.live.com/self.aspx/.Public/Blog%20Sample%20Code/ReadDirectoryChanges.zip)

之前，我花了一周时间研究文档少得可怜的 ReadDirectoryChangesW。希望这篇文章可以为大家节约一些时间。我相信我已经读过了我能找到所有相关文章和大量代码。几乎所有的例子，包括微软自己的那个例子，都有明显缺陷或低级错误。

我曾在《Multithreading Applications in Win32》这本书中的某一章，介绍了同步IO，激发态内核对象，重叠IO，IO完成端口的区别。现在要谈的这个问题，对我来说是小菜一碟。只不过上次写重叠IO的痛苦折磨了我好多年，这次应该也不会例外。


## 监控文件和目录的四种方式 ##

我们先看一下 SHChangeNotifyRegister，这个函数通过窗口消息实现，所以需要一个窗口句柄。它由Shell (Explorer)驱动，所以应用程序只会接收到 Shell 关心的通知，这些通知很难满足你的需求。它仅仅对监控用户对Explorer的操作有用。

在 Windows Vista 中，SHChangeNotifyRegister 已经可以报告所有文件的所有变更。但问题是，还存在上亿不打算立即升级的 Windows XP 用户。

由于 SHChangeNotifyRegister 基于窗口消息，所以还会带来性能上的问题。如果发生了太多文件变更，应用程序会不断接收到变更消息，你必须自己确认实际发生的事情。对于一部分应用程序来说，这实在是相当的囧。

Windows 2000 引入了两个新接口，[FindFirstChangeNotification](http://msdn.microsoft.com/en-us/library/aa364417%28VS.85%29.aspx) 和 [ReadDirectoryChangesW](http://msdn.microsoft.com/en-us/library/aa365465%28v=VS.85%29.aspx)。 FindFirstChangeNotification 很容易使用，但没有给出变更文件的信息。即便如此，这个函数对某些应用程序还是很有用的，比如传真服务和 SMTP 服务可以通过拖拽一个文件到一个目录来接受任务队列。ReadDirectoryChangesW 会给出变更的内容和方式, 不过相对的，在使用上也更复杂一些。

同 SHChangeNotifyRegister 一样，这两个新函数也会有性能问题。与 Shell 通知相比，它们的运行速度有明显提升，但在不同目录间移动上千个文件仍然会导致你丢失一部分(或者很多)通知。丢失通知的原因很[复杂](http://social.msdn.microsoft.com/forums/en-US/netfxbcl/thread/4465cafb-f4ed-434f-89d8-c85ced6ffaa8/)。令人惊讶的是，似乎与你处理通知的速度有关。

**注意**，FindFirstChangeNotification 和 ReadDirectoryChangesW 是互斥的，不能同时使用。

Windows XP 引入了最终解决方案，[变更日志(Change Journal)](http://msdn.microsoft.com/en-us/library/aa363803%28VS.85%29.aspx)可以跟踪每一个变更的细节，即使你的软件没有运行。很帅的技术，但也相当难用。

第四个，同时也是最后一个解决方案需要安装[文件系统过滤驱动](http://msdn.microsoft.com/en-us/library/ff548202.aspx)，Sysinternals 的 FileMon 就使用了这种技术。在 Windows 驱动开发包(WDK)中有一个例子。这个方案本质上是一个[设备驱动](http://msdn.microsoft.com/en-us/library/ff548084.aspx)，如果没有正确的实现，有可能导致系统稳定性方面的问题。

对我来说，使用 ReadDirectoryChangesW，在性能和复杂度上会是一个很好的平衡。


## 谜题 ##

使用 ReadDirectoryChangesW 的最大挑战在于，在IO模式，处理信号，等待方式，以及线程模型这几个问题的整合上，存在数百种可能性。如果你不是 Win32 I/O 方面的专家，即使最简单的场景，你也很难搞定。

  * A. I/O模式:
    1. 阻塞同步(Blocking synchronous)
    1. 触发式同步(Signaled synchronous)
    1. 重叠异步(Overlapped asynchronous)
    1. 完成例程(Completion Routine) (又名 Asynchronous Procedure Call or APC)

  * B. 当调用 WaitForXxx 函数的时候:
    1. 等待目录句柄
    1. 等待 OVERLAPPED 结构体里的 Event 对象
    1. 什么都不等 (APCs)

  * C. 处理通知:
    1. 阻塞
    1. WaitForSingleObject
    1. WaitForMultipleObjects
    1. WaitForMultipleObjectsEx
    1. MsgWaitForMultipleObjectsEx
    1. IO完成端口(I/O Completion Ports)

  * D. 线程模型:
    1. 每个工作线程调用一次 ReadDirectoryChangesW.
    1. 每个工作线程调用多次 ReadDirectoryChangesW.
    1. 在主线程上调用多次 ReadDirectoryChangesW.
    1. 多个线程进行多个调用. (I/O Completion Ports)

最后，当调用 ReadDirectoryChangesW 的时候，你可以通过 flags 选择你要监控的内容，包括文件创建，内容变更，属性变更等等。你可以多次调用，每次一个 flag，也可以在一次调用中使用多个 flag。多个 flag 总是正确的解决方案。但如果你为了调试方便，需要一个 flag 一个 flag 的调用的话，那就需要从 ReadDirectoryChangesW 返回的通知缓冲区中读取更多的数据。

如果你的脑子正在囧的话，那么你就能够明白为什么那么多人都没法搞定这件事了。


## 建议的解决方案 ##

那么正确的答案是什么呢？我的建议是：取决于你认为最重要的是什么。

**简单** - A2C3D1 - 在单独的线程中调用 ReadDirectoryChangesW，然后通过 PostMessage 发送给主线程。对于性能要求不高的 GUI 程序最合适。在 CodeProject 上的 [CDirectoryChangeWatcher](http://www.codeproject.com/KB/files/directorychangewatcher.aspx) 就是使用的这个策略。微软的 [FWATCH 例子](http://www.experts-exchange.com/Programming/Languages/CPP/Q_22507220.html) 也是使用的这个策略。

**性能** - A4C6D4 - 性能最好的解决方案是使用I/O完成端口，但是，这个激进的多线程方案实在太过复杂，应当仅限在服务器上使用。对任何 GUI 程序来说，这个方案似乎都是不必要的。如果你不是一个多线程专家，请远离这个策略。

**平衡** - A4C5D3 - 通过完成例程(Completion Routines)，在一个线程中完成所有工作。你可以发起尽可能多的 ReadDirectoryChangesW 调用，由于完成例程是自动分派的，所有不需要等待任何句柄。你可以通过回调传递对象的指针，以便跟踪原始的数据结构。

起初我曾经认为 GUI 程序可以通过 MsgWaitForMultipleObjectsEx 将变更通知混入到窗口消息中。但由于对话框有自己的消息循环，当对话框显示的时候，通知便无法处理了。于是这个好主意被现实无情的碾碎了。


## 错误的技术 ##

在研究解决方案的时候，我见识过各种用法：不靠谱的，错误的，以及错得离谱的。

如果你正在使用上面提到的简单方案，不要使用阻塞调用，因为唯一取消调用的方法是关闭句柄(未在文档中列出的方法)，或者调用 Vista 之后的函数 [CancelSynchronousIo](http://msdn.microsoft.com/en-us/library/aa363794%28VS.85%29.aspx)。正确的办法是使用触发式的同步I/O模式，也就是等待目录句柄。结束线程的时候，不要使用 TerminateThread，因为这个时候，资源无法释放，从而导致各种各样的问题。而是创建一个手动重置的 Event 对象，作为 WaitForMultipleObjects 等待的第二个句柄。当 Event 被设置的时候，退出线程。

如果你有上千个目录需要监控，不要使用简单方案。转换为平衡方案。或者监控公共的根目录，并忽略不关心的文件。

如果你需要监控整个驱动器，请三思。你会接收到每个临时文件，每个Internet缓存文件，每个应用程序数据变更的通知。简单来说，大量的通知会拖慢整个系统。如果你需要监控整个驱动器，你应当使用变更日志(Change Journal)。这样即使你的程序没有运行，也可以跟踪每一个变更。绝对不要用 FILE\_NOTIFY\_CHANGE\_LAST\_ACCESS 标志监控整个驱动器。

如果你使用了不带I/O完成端口的重叠I/O，不要等待句柄，而是使用完成例程(Completion Routines)。这样可以不受64个句柄的限制，可以让操作系统处理调用的分发，还可以通过 OVERLAPPED 传递你自己的对象指针。等一下我会给出例子。

如果你使用了工作线程，将结果传回给主线程的时候，不要使用 SendMessage，而是使用 PostMessage。如果主线程很繁忙，同步的 SendMessage 需要很久才能返回。这就失去了使用工作线程的意义了。

通过提供较大的缓冲区来尝试解决丢失通知的问题，会是一个诱人的选项。但这不是明智的行为。不管给定的缓冲区体积是多少，内核的未分页内存池都是分配相同大小的缓冲区。如果你分配太大的缓冲区，有可能导致包括蓝屏在内的一系列问题。感谢 MSDN 社区内容的匿名投稿人。


## 获取目录句柄 ##

现在我们来看看之前提到平衡方案的实现细节。在 ReadDirectoryChangesW 的声明中，你会注意到第一个参数是一个目录的句柄。你是否知道你可以获得一个目录的句柄呢？名为OpenDirectory的函数是不存在的，CreateDirectory也不会返回句柄。第一个参数的文档是这样描述的：”这个目录必须以 FILE\_LIST\_DIRECTORY 访问权限打开“。在后面的 Remarks 节提到：”要获取目录的句柄，需要以 FILE\_FLAG\_BACKUP\_SEMANTICS flag 调用 CreateFile 函数。“实际的代码如下：
```
HANDLE hDir = ::CreateFile(
    strDirectory,           // 文件名的指针
    FILE_LIST_DIRECTORY,    // 访问(读/写)模式
    FILE_SHARE_READ         // 共享模式
        | FILE_SHARE_WRITE
        | FILE_SHARE_DELETE,
    NULL, // security descriptor
    OPEN_EXISTING,         // 如何创建
    FILE_FLAG_BACKUP_SEMANTICS // 文件属性
         | FILE_FLAG_OVERLAPPED,
    NULL);                 // 文件属性的模板文件
```

第一个参数, FILE\_LIST\_DIRECTORY, 甚至没有在 [CreateFile() 的文档](http://msdn.microsoft.com/en-us/library/aa363858%28VS.85%29.aspx)中提到。而是在[文件安全和访问权限(File Security and Access Rights)](http://msdn.microsoft.com/en-us/library/aa364399%28v=VS.85%29.aspx)中有一些没什么用的描述。

类似的，FILE\_FLAG\_BACKUP\_SEMANTICS 有这样一行有趣的标注：“如果此标志没有与 SE\_BACKUP\_NAME 和 SE\_RESTORE\_NAME一起使用，仍然会进行适当的安全检查。”在我过去的印象中，使用这个标志需要管理员权限。这个标注证实了这一点。不管怎样，在 Windows Vista 系统中，如果启用了 UAC，调整安全令牌以启用这些权限的操作是不管用的。这里，我不确定到底是要求改变了，还是文档有歧义。其他类似的内容也令人困惑。

共享模式也存在一个陷阱，我看到一些例子没有使用 FILE\_SHARE\_DELETE。也许你认为目录不会被删除，所以没有问题。但是，这回导致其他进程无法重命名或者删除这个目录下的文件

这个函数另一个潜在的陷阱在于，被引用的目录本身处于”使用中“的状态，并且无法被删除。如果希望在监控目录的同时，还允许目录被删除，你应当监控该目录的父目录及父目录下的文件和子目录。

## 调用 ReadDirectoryChangesW ##

实际调用 ReadDirectoryChangesW  是整个操作中最简单的环节。如果你使用了完成例程，唯一需要注意的就是缓冲区必须是DWORD对齐的。

OVERLAPPED 结构体用来指定重叠操作，但实际上 ReadDirectoryChangesW 没有使用结构体中的任何一个字段。关于完成例程，这里有一个大家都知道的小技巧，就是你可以提供一个C++对象的指针。文档是这么说的：”OVERLAPPED 结构的的 hEvent 成员不会被系统使用，所以你可以按自己的方式使用。“这意味着你可以将你自己对象的指针放进去。你可以在下面的示例代码中看到这一点：
```
void CChangeHandler::BeginRead()
{
    ::ZeroMemory(&m_Overlapped, sizeof(m_Overlapped));
    m_Overlapped.hEvent = this;

    DWORD dwBytes=0;

    BOOL success = ::ReadDirectoryChangesW(
        m_hDirectory,
        &m_Buffer[0],
        m_Buffer.size(),
        FALSE, // monitor children?
        FILE_NOTIFY_CHANGE_LAST_WRITE
         | FILE_NOTIFY_CHANGE_CREATION
         | FILE_NOTIFY_CHANGE_FILE_NAME,
        &dwBytes,
        &m_Overlapped,
        &NotificationCompletion);
}
```

由于我们使用了重叠I/O，m\_Buffer直到完成例程被调用的时候才会填充。


## 分派完成例程 ##

对于我们讨论的平衡方案，有两个方法等待完成例程被调用。如果所有分派都使用完成例程，那么只需要 SleepEx就可以。如果你需要在分派完成例程的同时等待句柄，那么你需要使用 WaitForMultipleObjectsEx。这个函数的Ex版本要求将线程置为 "alertable" 状态，"alertable"状态指完成例程将要被调用。

如果要结束使用SleepEx的线程，你可以设置一个 SleepEx 循环中的标记，以退出SleepEx 循环。如果调用完成例程，你可以使用QueueUserAPC，这个函数允许一个线程调用另一个线程中的完成例程。


## 处理通知 ##

通知例程很简单，只要读取数据并保存就可以了。真的是这样么？错。完成例程的实现也有其复杂度。

首先，你需要检查并处理错误码 ERROR\_OPERATION\_ABORTED，这个错误码意味着 CancelIo 被嗲用，这是最后的通知，你需要做合适的清理工作。CancelIo的更多细节会在下一节描述。在我的实现中，我使用 InterlockedDecrement 来减少 cOutstandingCalls 的值，这个变量用来跟踪活动调用的计数，然后返回。我的对象都由 MFC 框架进行管理，所以不需要再完成例程中释放。

你可以在单次调用中接收多个处理。务必遍历数据结构，并挨个检查非空的 NextEntryOffset 字段

ReadDirectoryChangesW 是一个 "W"例程，所以它使用Unicode。这个例程没有 ANSI 版本。因此，数据缓冲区自然也是Unicode。字符串不是 NULL 结尾的，所以你不能使用 wcscpy。如果你使用 ATL 或 MFC 的 CString 类，你可以
用原始字符串加上给定的数字来实例化一个宽字符的CString
```
FILE_NOTIFY_INFORMATION* fni = (FILE_NOTIFY_INFORMATION*)buf;
CStringW wstr(fni.Data, fni.Length / sizeof(wchar_t));
```

最后，你必须在退出完成例程前，重新发起 ReadDirectoryChangesW 的调用。你可以重用相同的 OVERLAPPED 结构体。文档指出，在完成例程被调用后，OVERLAPPED 结构体不会再次被 Windows使用。但是，你必须确保缓冲区与当前调用使用的缓冲区不同，否则会遇到“竞态条件”。

有一点我不太清除，那就是在完成例程被调用和发起新的 ReadDirectoryChangesW 调用之间，变更通知做了什么事情。

我还必须重申，如果很多文件在短时间发生变更，你有可能丢失通知。根据文档描述，如果缓冲区溢出，整个缓冲区的内容都会被丢弃，lpBytesReturned会返回0。但是我不清除完成例程是否会将 dwNumberOfBytesTransfered 为 0 ，或者是否会将 dwNumberOfBytesTransfered 指定为错误码。

有几个关于完成例程错误实现的有趣例子。我最喜欢的一个是在 [stackoverflow.com](http://stackoverflow.com/questions/342668/how-to-use-readdirectorychangesw-method-with-completion-routine)上找到的。那个家伙在喷完一个求助帖后，展示了他自己的完成例程实现，并叫嚣：”这玩意看起来也不难嘛“。他的代码漏掉了错误处理，他没有处理 ERROR\_OPERATION\_ABORTED，没有处理缓冲区溢出，他甚至没有重新发起 ReadDirectoryChangesW 调用。我觉得，如果忽略了这些困难的事情，剩下的，的确没什么难的。


## Using the Notifications ##

当你接受并解析一个通知时，你需要确定如何处理它。这并不容易。首先，你将经常接收到多个重复的变更通知，特别是一个进程在写入一个大文件时。如果要等待文件的写入完成，你需要等待直到一段时候内都不再有文件更新之后，才能开始进行处理。

[Eric Gunnerson](http://blogs.msdn.com/ericgu/archive/2005/10/07/478396.aspx) 的一篇文章指出，FILE\_NOTIFY\_INFORMATION的文档有一个关键的描述：如果文件既有长文件名，又有短文件名，那么文件会返回其中的一个名字，但不确定返回哪一个。大多数时候，在短文件名和长文件名之间转换都很容易，但是文件被删除后，就不一样了。因此，你必须维护一个跟踪文件的列表，同时跟踪长文件名和短文件名。我无法在 Windows Vista 上重现这个行为，不过我只在一台计算机上做过尝试。

你有可能接收到你没有预料到的通知。例如，即使你设置了 ReadDirectoryChangesW 的不接收子目录通知的参数，你仍然会接收到子目录本身的通知。假设你有两个目录 C:\A 和 C:\A\B。你将文件 info.txt 从第一个目录移动到第二个目录。你将会接收到 C:\A\info.txt 的 FILE\_ACTION\_REMOVED  通知，以及 C:\A\B 的 FILE\_ACTION\_MODIFIED 通知。不过，你不会接收到任何关于 C:\A\B\info.txt 的通知。

令人惊讶的事情还会发生。你是否使用过 NTFS 的硬链接？硬链接允许你将多个文件名引用同一个物理文件。如果你在一个目录中监控一个引用，在另一个目录中监控另一个引用，当修改第二个目录中的文件时，会生成第一个目录的通知。灰常的神奇。

另一方面，如果你使用Windows Vista引入的符号链接，被链接的文件不会生成通知。仔细想想，也说得过去，但是你得小心各种各样的可能性。

还有第三种可能，就是 Junction 从一个分区链接到另一个。这种情况下，对子目录的监控不会监控被链接分区中的文件。这种行为也说得通，但是当发生在用户的机器上时，这种现象会令人感到困惑。


## 关停 ##

我没有找到任何文章和代码（即使在开源代码中）适当的清理了重叠调用。MSDN文档 指出通过调用 CancelIo 来取消重叠I/O。这很容易。但是，我的应用程序退出的时候会崩溃。堆栈显示，我的某个第三方库正在将线程置为 'alertable' 状态（意即可以调用完成例程了），并且即使在我调用了CancelIo，关闭了句柄，删除了 OVERLAPPED  结构体之后，我的完成例程还是被调用了。

于是我搜索了各种各样的关于调用 CancelIo 的网页，我找到[这个网页](http://cboard.cprogramming.com/windows-programming/111826-readdirectorychangesw-again.html) 中包含这样的代码：
```
CancelIo(pMonitor->hDir);

if (!HasOverlappedIoCompleted(&pMonitor->ol))
{
    SleepEx(5, TRUE);
}

CloseHandle(pMonitor->ol.hEvent);
CloseHandle(pMonitor->hDir);
CancelIo(pMonitor->hDir);
```

这个看起来很有希望成功，我信心满满得把这段代码拷贝到我的程序中，但是不管用。

我再次查阅了 CancelIo 的文档，其中指出，”所有被取消的I/O操作都会以ERROR\_OPERATION\_ABORTED 错误结束，并且所有的I/O完成通知都会正常发生。“换句话说，在CancelIo被调用后，所有的完成例程都都至少会被调用最后一次。对 SleepEx 的调用也本该允许，但不是这样子。最后我认为，等待5毫秒太短了。也许将"f"改成"while"就能解决这个问题了，但是这个方案要求轮询每一个重叠结构体，于是我选择了不同的方式。

我最终的解决方案是跟踪未完成的请求数目，然后持续调用 SleepEx 直到计数为0，在示例代码中，关停的顺序如下：

  1. 程序调用 CReadDirectoryChanges::Terminate (或者简单的析构对象)
  1. Terminate 通过 QueueUserAPC 发送消息到工作线程中的 CReadChangesServer，通知其结束。
  1. CReadChangesServer::RequestTermination 将 m\_bTerminate 设置为 true，然后将调用转发给 CReadChangesRequest 对象，每个对象对自己的目录句柄调用 CancelIo 然后关闭目录句柄。
  1. 控制返回到 CReadChangesServer::Run 函数，注意这时还没有任何东西实际结束。
```
void Run()
{
    while (m_nOutstandingRequests || !m_bTerminate)
    {
        DWORD rc = ::SleepEx(INFINITE, true);
    }
}
```
  1. CancelIo 导致 Windows 自动对每一个 CReadChangesRequest 重叠请求调用完成例程。每个调用的 dwErrorCode 都被设置为 ERROR\_OPERATION\_ABORTED。
  1. 完成例程删除 CReadChangesRequest 对象，减少 nOutstandingRequests 计数，然后在不发起新请求的情况下返回。
  1. 由于一个或多个APCs完成，SleepEx返回。nOutstandingRequests 为0，bTerminate 为true，于是函数退出，线程被干净的结束。

万一关停没有被合适的处理，主线程会根据一个超时时间等待工作线程结束。如果工作线程没有顺利结束，我们就让 Windows 结束时干掉它。


## 网络驱动器 ##

ReadDirectoryChangesW 可以使用在网络驱动器上，当且仅当远程服务器支持这个功能。从基于Windows的计算机共享的目录可以正确的生成变更通知。 Samba 服务器则不会生成通知，大概因为相关操作系统不支持这个功能。网络附加存储（NAS）设备通常运行Linux系统，所以也不支持通知。至于高端存储域网络（SANs），那就谁也说不准了。

ReadDirectoryChangesW 当缓冲区长度大于 64 KB 并且程序监控网络上的一个目录时，会失败并返回错误码 ERROR\_INVALID\_PARAMETER。这是因为相关的网络共享协议对包大小有限制。


## 总结 ##

如果你看到了这里，我要为你的"can-do"态度鼓掌。希望你清晰的了解了如何使用ReadDirectoryChangesW，以及为什么要怀疑你看到的所有关于这个函数的示例代码。仔细的测试很关键，也包括性能测试。