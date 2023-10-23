<h1 align="center">
  <a href="https://deliciouslines.com/one-file-docs" target="_blank">
    <img width="50%" src="https://deliciouslines.com/images/ofd-logo-hires.png"/>
  </a>
</h1>

<h2 align="center">Generate a single HTML file from markdown, right away.</h2>

**[Download latest realease](https://github.com/deliciouslines/one-file-docs/releases/latest)**

### Features:
- No dependencies, no installs, download it and run it out of the box
- No Javascript
- Human readable header IDs
- Custom themes
- Build documentation from individual files or from a directory
- Use it as a single-file C/C++ library

**Basic usage:**
- From individual files: `ofd file1.md file2.md file3.md -o result.html`
- From a directory: `ofd -d documentation -o result.html`

**Full documentation**: https://deliciouslines.com/one-file-docs

### How to build
Use one of the `build_*` scripts for your platform and compiler.
If you cannot find a build script for your compiler/OS, give `entry_point.c` as input to your compiler of choice.
You need to link against the C runtime and on Windows you also need to link against `Kernel32` (if your compiler supports `#pragma comment(lib)` directives then it is already done for you in the source code).

### License

One-File Docs is placed under the MIT license.
One-File Docs uses STB sprintf which is also placed under the MIT license.

### Bug reports and feature requests

Feel free to post an issue if there is anything you would like to see implemented or if you encounter problems.

### Markdown support:
- Headers
- Emphasis
- Strike-through
- Horizontal rules
- Links (auto-linking works, you can also embed images inside links)
- Images (additional options are available for width, height and rounding)
- Ordered and unordered lists
- Todo-lists
- Tables
- Quotes (cannot be nested at the moment)
- Code blocks (including the ones with syntax highlighting)
- Embedded HTML
