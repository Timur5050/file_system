# Movivation

# How to run
```sh
mkdir build && cd build
cmake ..
make
./my_fs
```

# CLI commands available
#### navigation commands
- pwd - prints the current directory path
- ls - lists files and directories in the current directory
- cd `<dir>` - changes the current directory to `<dir>`
- exit - exits the CLI
#### directory management
- mkdir `<dir>` - creates a new directory
- rmdir `<dir>` - removes an empty directory
- rm -r `<dir>` - removes directory along with all its contents
- rm `<file>` - deletes file
#### file management
- touch `<file`	creates a new file
- cat `<file>` displays the content of a file
#### text handling
- echo `"<text>"`	prints <text> to the console
- echo `"<text>"` > <file> overwrites <file> with <text>. creates file if it doesn't exist
- echo `"<text>"` >> <file>	appends <text> to <file>
