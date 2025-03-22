# Movivation
When I was deciding what to study next after backend development, I came across something that is now called “computer science basics” on the Internet. I was very interested and impressed by how broad the field of computer technology is. The first thing I decided to do after creating a plan was to learn about the computer, how it works, computer architecture, and everything related to computer systems. It was a really amazing course from UC Berkeley, known as CS 61C. But it wasn't enough to understand how a computer works, so I continued my journey by studying the next topic in the plan: operating systems. I decided to study OS using the Wisconsin-Madison textbook/book: Three Easy Parts. This is an amazing course and the best basic knowledge about OS. When I read the last “piece” of this book: perseverance. I was very impressed with the workings of file systems and how the OS manages disks, SSDs, and all other persistent devices. So I decided to read more about it in other articles, books, and resources. It got me interested and I had the idea to write my own UNIX-based file system with some small custom details that I developed. One interesting addition was to add some CLI commands to make it easier to test the FS and find more bugs.





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
