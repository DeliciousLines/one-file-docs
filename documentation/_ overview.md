# Download

You can download the latest release from [Github](https://github.com/deliciouslines/one-file-docs/releases/latest).

# Goal And Philosophy

One-File Docs takes one or several markdown files and generates a single HTML file.
The goal is to have a tool that is as lightweight as possible with as few dependencies as possible. All you should have to do is download the executable and use it right away without having to install anything.
One-File docs is also accessible as a single-file C library that should be very easy to integrate with your codebase and work with all major C/C++ compilers.

## Who is this for
The goal here is simplicity. If you only want a single HTML file containing documentation without having to deal with multiple files and folder hierarchies, then this tool is for you.
If you are also looking for a tool that just works on its own without having to download packages or whatnot, you knocked on the right door.

# Quick start

Basic usage:
`ofd file1.md file2.md file3.md -o result.html`

Specifying a folder instead:
`ofd -d documentation -o result.html`.
[Learn more about building documentation from folders.](#Building-documentation-from-a-folder)

# Command-line options

| *Switch*        | *Argument* | *Description*
| `-o`, `-output` | file       | Specifies the output filepath. The default is `result.html`.
| `-t`, `-theme`  | file       | Specifies a [theme file](#Themes). If no theme is provided a default theme is applied.
| `-l`, `-logo`   | path       | Specifies a path to a logo.
| `-i`, `-icon`   | path       | Specifies a path to an icon.
| `-title`        | title      | Specifies a title.
| `-c`, `-config` | file       | Specifies a [project configuration file](#Project-configuration-files).
| `-d`, `-dir`    | directory  | Specifies a [directory to build documentation from](#Building-documentation-from-a-folder). Note that you cannot specify markdown files if you use this option.
| `-h`, `-help`   |            | Shows a help message.

# Configuration file

A configuration file is a simple text file that contains fields. You can specify one line per field. Example:

``
# This is a comment.

text_colour  1 1 1 1
font_size    2
``

Right now there are 3 types of fields:
- Colour fields
- Real number fields
- String fields

## Colours
[colour_type]: #Configuration-file-Colours
\
A colour is composed of 4 real numbers representing red, green, blue and alpha. All values must be in [0;1]. Example:
`text_colour  .5 1 0.2 1`

## Real numbers
[f32_type]: #Configuration-file-Real-numbers
\
The way you specify a real number is straightforward:
`font_size 2`

## Strings
[string_type]: #Configuration-file-Strings
\
Strings are enclosed in quotes. Note that for now you cannot escape characters in a string. Example:
`logo  "https://website.com/images/logo.png"`

# Themes

> **NOTE**
> To specify a theme on the command line use the `-t` or `-theme` switch, e.g. `ofd -t theme.txt <...>`.

A theme is a [configuration file](#Configuration-file) where you can specify the following fields:

| *Name*                           | *Type*                  | *Comment*
| `background_colour`              | [colour][colour_type]   |
| `text_colour`                    | [colour][colour_type]   |
| `code_block_background_colour`   | [colour][colour_type]   |
| `code_block_text_colour`         | [colour][colour_type]   | This may be overrided in certain contexts (e.g. in quotes).
| `link_colour`                    | [colour][colour_type]   | This may be overrided in certain contexts (e.g. in quotes).
| `image_title_colour`             | [colour][colour_type]   |
| `horizontal_rule_colour`         | [colour][colour_type]   |
| `quote_background_colour`        | [colour][colour_type]   |
| `quote_text_colour`              | [colour][colour_type]   |
| `section_link_text_colour`       | [colour][colour_type]   |
| `section_link_background_colour` | [colour][colour_type]   |
| `separator_colour`               | [colour][colour_type]   |
| `scrollbar_colour`               | [colour][colour_type]   | This is not supported by all browsers.
| `font_size`                      | [real number][f32_type] | In % of the render target height (uses `vh` internally).
| *The following fields are only relevant in code blocks.*
| `syntax_comment_colour`          | [colour][colour_type]   |
| `syntax_string_colour`           | [colour][colour_type]   |
| `syntax_number_colour`           | [colour][colour_type]   |
| `syntax_structure_colour`        | [colour][colour_type]   | e.g. `{`, `;`, `]`, etc.
| `syntax_keyword_colour`          | [colour][colour_type]   |
| `syntax_directive_colour`        | [colour][colour_type]   |

# Project configuration files

> **NOTE**
> To specify a project configuration file on the command line use the `-c` or `-config` switch, e.g. `ofd -c config.txt <...>`.
> Usually project configuration files are used in place of command line switches.

A project configuration file is a [configuration file](#Configuration-file) where you can specify the following fields:

| *Name*   | *Type*                | *Example*
| `title`  | [string][string_type] | `"My App's Documentation"`
| `logo`   | [string][string_type] | `"https://website.com/logo.png"`
| `icon`   | [string][string_type] | `"https://website.com/icon.png"`
| `theme`  | [string][string_type] | `"theme.txt"`
| `output` | [string][string_type] | `"result.html"`

# Building documentation from a folder

> **NOTE**
> To specify a folder use the `-d` or `-dir` switch, e.g. `ofd -d documentation`

When you build documentation from a folder all `.md` files within the folder are used as input. Note that sub-folders are **not** taken into account.
If you like to work with [project configuration files](#Project-configuration-files) you can also add one in the folder, just make sure you name it `.ofd` and it will automatically be used.

> **NOTE**
> All paths within the `.ofd` project file are relative to the folder (except for the `logo` and `icon` fields).
> For instance if you want to use `folder-path/theme.txt` as a theme then you can simply specify `theme "theme.txt"` in the `.ofd` file.

## Overriding `.ofd`

You can easily override values from the `.ofd` file by providing them on the command line.
For example if you want to output documentation to a different file you can run `ofd -d documentation -o alternative_path.html`.

## File order

When you specify markdown files on the command-line they are added in order to the HTML file. Since here we are loading them from a folder, the order would depend on the OS.
To mitigate this and introduce consistency all `.md` files are sorted by name in ascending order. This lets you name them accordingly so that they appear in the correct order.
One trick you can use is to prepend '\_' characters to the file name. Since '\_' characters are replaced by spaces and all leading spaces are removed, this will not have an impact on the way the file name looks like in the generated documentation. This will however have an impact on file order.
For example you could have the following files:

``
_ file1.md
__ file2.md
___ file3.md
``

# Markdown

One-File Docs supports most Markdown features and has a few of its own. Some features are currently not available but will likely be added in the future.

## Supported features

The following only lists notable features.

### Escaping characters

You can use `\` to escape a character. This also works on new line characters.

### Links
You can use the usual syntax for links:
`[link text](<address> "<title>")`

You can use markdown for the link text.

> **NOTE**
> There is partial support for reference-style links, i.e. `[link name][reference ID]`. If you want to use a reference you need to define it before you use it.
> A link which reference cannot be found will not be formatted.

### Images
The usual image syntax is supported with additional options:
`![Alternative text](<address> "<image title>" <options>)`

You can use markdown in the image title.

> **NOTE**
> You can use the reference-style syntax as well provided that you define the reference beforehand.

#### Additional options
| *Syntax*      | *Description*
| `w="<value>"` | This lets you specify a width. You can use CSS syntax for this.
| `h="<value>"` | This lets you specify a height. You can use CSS syntax for this.
| `r="<value>"` | This lets you specify rounding (`border-radius`). You can use CSS syntax for this.

### Tables

The `|` character represents a table cell. If there is an empty line between two rows then you get two different tables. Example:
``
| *Colum 1* | *Column 2*
| value     | value
| value     | value
| value     | value

| Other table
| value
| value
| value
``

You can use markdown within table cells.

### Embedded HTML
You can easily embed HTML using `</>`. Example:

``
</>
<div style='width: 100%; height: 10%; background-color: lightgreen;'></div>
</>
``

This produces the following:
</>
<div style='width: 100%; height: 10%; background-color: lightgreen;'></div>
</>

### Code blocks
The usual \` syntax can be used for code blocks. One-File Docs also supports code blocks with syntax highlighting using the following syntax:

``
```<language ID>
<code>
```
``

List of supported language IDs:
| *ID*               | *Language*
| `c`, `cpp`, `c++`  | C/C++
| `rs`, `rust`       | Rust
| `go`               | Go
| `cs`, `c#`         | C#
| `java`             | Java
| `jai`              | JAI
| `perl`             | Perl
| `sh`, `bash`       | Bash script
| `bat`              | Windows shell script
| `py`, `python`     | Python
| `js`, `javascript` | Javascript
| `ts`, `typescript` | Typescript
| `php`              | PHP
| `json`             | JSON
| `xml`, `html`      | XML/HTML
| `css`              | CSS

> **NOTE**
> You can customise syntax colours by setting the `syntax_*` fields in your [theme file](#Themes).

## Unsupported features

> **NOTE**
> If one of the features below is something you really need you can [embed HTML](#Markdown-Supported-features-Embedded-HTML) to reproduce it.

### Underlined headers
You cannot specify a header in the following way:
``
Header
======
``

This feature is very unlikely to be added in the future.

### Nested notes
If enough people show interest in this it will be added in the future but right now this is not supported and there are no plans to support it.

### Nested lists
Technically you can add nested lists but they will not appear the way they are supposed to. This will be added in the future.

### Ordered list
You can still use the notation but the result is not styled in any specific way. This will be added in the future.

### Todo-lists
Todo-lists will probably be added in the future. For now the following has no effect:

``
- [x] Something
- [ ] Something else
- [ ] Something else entirely
``

# How to build

We currently provide binaries for Windows (x64), Linux (x64) and macOS (x64, ARM). We also provide shell scripts for building the program with major compilers.
If you want to build it yourself without using one of the provided options, all you have to do is give `entry_point.c` to your compiler of choice.
Make sure you also link against the C runtime.
On Windows make sure to link against `Kernel32`.

# Using One-File Docs as a library

You can use One-File Docs as a C/C++ library by including `ofd.h` in your project. All you have to do is to make sure you define `OFD_IMPLEMENTATION` *once* in your codebase before including `ofd.h`.
For additional information check `ofd.h`.
Check `entry_point.c` if you want to see it in action.

# License

One-File Docs is placed under the MIT license.

Copyright (c) 2023 Delicious Lines

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

**THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.**