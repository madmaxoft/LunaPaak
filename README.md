# LunaPaak
A whole Lua in a single Windows executable.

The point of this project is to provide a Lua interpreter with some useful libraries, in a single executable. The main intended audience are people with zero programming experience to be able to run advanced Lua scripts on Windows. No need to install LuaRocks or LuaDist, no fiddling with compilers, dependencies etc.

The package contains the following libraries:
- Lua interpreter
- luafilesystem
- luasocket
- lsqlite (+ sqlite)
- lzlib (+ zlib)
- lua-cjson
- LuaSimpleWinHttp
- [LuaExpat](https://lunarmodules.github.io/luaexpat/manual.html#parser)

# Installing
To install the interpreter, download the executable from the Releases, save it to a final destination and execute it once. It will register itself to handle .luna files automatically, and will add itself to the list of programs capable of handling .lua files. Next, you can run any .luna file by dbl-clicking it. For .lua files, you will need to manually set it as the default program, using built-in Windows functionality.

# About the license
The unlicense only applies to the wrapper program; each library has its own separate license.
