### xout
Cross-Application Output for Windows
---

What:
Unified output for Windows, the base of xout is a library that spawns a window used for outputting text; this resembles a terminal or console, but it's simply a windows containing a RichEdit 5.0 control to accept output.
Xout has a server and client component as well, you can run the xout server as part of any application or standalone, and it uses Windows' shared memory for communication. The client library can be integrated into any application or library, it's lightweight and header-only.

How:
Xout uses two streams (`std::ostream` & `std::wostream`) and combines the outputs, allowing you to mix narrow and wide (utf-16) strings.

Why:
Windows console is not fun to work with, requireing you to choose character width once at initialization. Simply; I don't like it for output.

Features:
- As mentioned, cross-application output.
- C++20 std::vformat style
- Full RGB Color support
- Simple and sensible defaults.

![Example](img\preview.png)



Icon from [Iconoir](https://iconoir.com/support), go support them!
