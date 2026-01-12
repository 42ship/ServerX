## Code Reading

question0: what are the best approach for people to read?
I guess there aren't best approach other than reading them one by one,

I thought about encoding by taking keywords down by typing them out, then asking
questions to see
here are my appraoch below



## Keywords

### src/http/internal/ChunkedBodyParser

- namespace http{...}
- ChunkedBodyParser{...}

public:
    - variable
        State {...}
    - function
        explicit ChunkedBodyParser(utils::TemFile &file);
        State reset();
        State feed(...)
        State state() const;
        HttpStatus errorStatus() const;
        void setMaxBodySize(...);

private:
    - State setErrpr(HttpStatus status);
    - State state_;
    - size_t maxBodySize_;
    - size_t bytesWritten_;
    - size_t chunkBytesLeft_;
    - HttpStatus errorStatus_;

then we have a class `TempFile` from the namespace `utils`
from the library `filesystem.hpp` in the `common` folder

so I had a look
```cpp
class TempFile {
public:
    TempFile();
    ~TempFile();

    bool open();
    void close();
    operator int() const;
    int fd() const;
    std::string const &path() const;
    bool isOpen() const;

private:
    int fd_;
    std::string filePath_;

    TempFile(TempFile const &TempFile);
    TempFile &operator=(TempFile const &rhs);
};

```

question1: where I don't understand `operator int() const`, what is this function?

question2: so I saw the copy constructor and copy assignment are in the private, i don't
understand why the coworker decided to do so; not that I have a better idea, but
more like are there any situations I don't know of and that situation makes them
in private as a better solution.

question3: how would a senior software engineer would navigate this situation?

my approach is as for now, I think I need to leave these alone, they are non member function
for me to understand for now.

```
bool writeFile(const std::string &content, const char *path);

bool isDir(const std::string &p);

const char *validateDirectoryPath(const char *path);

std::string getFileExtension(const std::string &fpath);

http::HttpStatus checkFileAccess(const std::string &path, int modeMask,
                                 bool allowDirectory = false);

std::string joinPaths(const std::string &p1, const std::string &p2);

```

the next ons is to have a look `HttpStatus`

for this reading, I would say the only thing i would need to know is

2xx is Success
3xx is Redirection
4xx is Client Error
5xx is Server Error


question4:I discoverd a gap in my concept , what the keyword `explicit` is?

and how weird it is, I also discoverd

the function name `explicit ChunkedBodyParser(utils::TempFile &file)` is the
same as `class ChunkedBodyParser{...}`


question5: any advanced to navigate via neovim editor? or how can I make my `ctags` smarter?! or are there a better way to jump
files between? or `ctags` doesn't work properly since `state()` actually is
repeated over the place in this C++ project

Context:
I would like to know is since I can't press `ctrl` + `[` to
jump into the file, so I did
```bash
ctags -R ./src ./inc
```

because I wanted to see what the implementation of `State state() const;`
then pressed `ctrl` + `[` when the cursor is hovering on `State state() const;`
but then it led me to the line 32 of code in the file
`./src/http/RequestParser.cpp` - `RequestParser::State RequestParser::state()
const {return state_;}`

so I can't see if that is the member function declared in

```cpp
class ChunkedBodyParser {
    ...
public:
    ...
    State state() const;
}
```

as of writing, I realised what mistakes I made, because of how 42 restricted how
we write C++98, then I got confused.

Since I was indeed looking for the implementation of `State state() const;`

so that line basically is saying
State is from the `class RequestParser {...}`
which is living in `inc/http/RequestParser.hpp`

Now I sort of connected with my knowledge about minishell
this state is actually sort of signal/flag to make code understand what the
execution context is.

But the only thing I am confused is seemly wrong for the `ctrl` + `[` because
the code as follow:
```
namespace http {
...
RequestParser::State    RequestParser::state() const {return state_;}
}
```
is the member function from `RequestParser`
rather than the class `class ChunkedBodyParser {... State state() const;}`


Since I know what to find so I got the file
`src/http/internal/ChunkedBodyParser.cpp`, I now know my previous mention was
correct,


question6: what are the good mental mdoels/habits to wrap my head around when
doing C++98 and in the future C++ advance/modern projects?

After recently studying C++98 fundamentals, I got better at my C++ skills.
I captured my mistakes. Initially, my brain was a bit fuzzy because when I read `RequestParser::State
RequestParser::state() const {return state_;}`, I was hesitant but when I slow
down my pace by writing them down, I realised it was because in `.cpp`, you need
to specify where a class, member function, variable from, so immediately I got
it even it did make the code reading experience suck.

question7: Are there no better way to do so or this is the nature of the C++98
project?

Context:
since the web server project written in C++98 by 3 different people,
hence the variable/function names can be repeated, or even so these are more
paradim/context shift coming from C99 backgrounds, it is time to adjust habits

when it comes to this, we need to know execution contexts, or class contexts
(please help me find a better way to say this), my guess is that I need to
develop some mental muscle to know RequestParser::State RequestParser::state()
{return state_;} is different from ChunkedBodyPraser::State
ChunkedBodyParser::state( return state_; ) which is verified by looking at these
two files.

Initially, I would say my the other two teammates were doing quite bad jobs,

because I thought there were no functional difference between

`ChunkedBodyParser::State ChunkedBodyParser::state() const {return state_;}` abd
`RequestParser::State RequestParser::state() const { return state_; }` but I
could be wrong because the state for request parser and state for chunked body
parser are different! so I take my comments back



question8:
okay I saw the comment
`//TODO: finish this implementation will modify state_ and bodyFile_ when
implemented`

but I don't know what that means

so this is the member function `feed`


question9:


initially, I didn't know if `ChunkedBodyParser` happened before the part handling request or
parsing incoming request?

```
        request
Client -----------> Server

Server
    - request parser
    - request handler
    - response build?
    - send response

```

until I saw your guide,

```
 | Order | File                                     | Why                                      | Lines    |
 |-------|------------------------------------------|------------------------------------------|----------|
 | 1     | inc/http/internal/ChunkedBodyParser.hpp  | Your interface - what you must implement | 45 lines |
 | 2     | src/http/internal/ChunkedBodyParser.cpp  | The stub you'll fill in                  | 33 lines |
 | 3     | src/http/RequestParser.cpp lines 140-146 | The ONE function that calls your code    | 7 lines  |
 | 4     | inc/common/filesystem.hpp lines 21-39    | TempFile - where you write output        | 18 lines |
```
Checked the `src/http/RequestParser.cpp`
```cpp
void RequestParser::handleChunkedBody() {
    LOG_ERROR(
        "RequestParser::handleChunkedBody(): Chunked parsing is not implemented. State=ERROR");
    ChunkedBodyParser::State chunkState = chunkParser_.feed(buffer_);
    (void)chunkState;
    setError(NOT_IMPLEMENTED);
}
```

question 10: without your guide, I 95% would be lost by reading all of the code,
and I might even be struggling and ended up with either getting overwhelmed or
doubting myself. but I learned that if I feel any of these feelings,  I must
focus on the wrong thing.

^begin
okay yes, in this part, now I know `RequestParser` --> `ChunkedBodyParser::State chunkState = chunkParser_.feed(buffer_)`

even so, now I know `chunkParser_` is being declared as a private member
variable from `class RequestParser{...}` because of the pattern `xxxx_` so then
^end

question11: is this direction of thinking better as this is really granular meta
thinking, please tell me the best practice. as of writing the part(check content
between
^begin...^end), I realised there could be two direction, I can first check
`buffer_`
- where is this private member variable `buffer_` from? by the fact there is no
  any namespace (question12: what term would you use to describe
`ChunkedBodyParser` from `ChunkedBodyParser::State` so people know what you are
talking about, as far as I know `::` is scoring qualifier. ), it belongs to the
`class RequestParser {...}`, if `buffer_` belongs to other classes, it would
have `xxxx::buffer_`
- What does this private member variable `buffer_` do?  by the reading it seems
  like an empty variable with memory allocated to be ready for taking some
values!

question12: Why couldn't I write the implementation of
`ChunkedBodyParser::feed()`? I must be blocked by something else?


I guess I could just write a similiar function? but I don't know why I don't
feel comfortable to write `ChunkedBodyParser::feed()`, even I think this very
resemble to

```cpp
RequestParser::State RequestParser::feed(char const *chunk, size_t size) {
    if (state_ == READING_HEADERS) {
        buffer_.append(chunk, size);
        parseHeaders();
    }
    if (state_ == READING_BODY) {
        buffer_.append(chunk, size);
        parseBody();
    }
    return state_;
}
```

maybe I just felt like I wasn't ready, or I got unclear, I also didn't feel it
would be the same after checked

```
class ChunkedBodyParser {
public:
    enum State {
        PARSING_CHUNK_SIZE,
        PARSING_CHUNK_DATA,
        PARSING_TRAILER,
        DONE,
        ERROR
    };

```

question13: after pasting out the code

```cpp
class ChunkedBodyParser {
public:
    enum State {
        PARSING_CHUNK_SIZE, // Waiting for size (e.g., "A0\r\n")
        PARSING_CHUNK_DATA, // Reading the N bytes of data
        PARSING_TRAILER,    // Reading the final '\r\n' after the data
        DONE,               // Saw "0\r\n\r\n"
        ERROR
    };
```

I felt like I should design the state, but also generally I didn't know what I
needed to do to progress.

question14: How should I think about the problem without you as my assistant?

```bash
Network (you know this)
      ↓
      ↓ raw bytes arrive
      ↓
  RequestParser.feed()     ← calls your code when chunked
      ↓
      ├─→ handleContentLengthBody()   ← not your problem
      └─→ handleChunkedBody()         ← YOUR ENTRY POINT
              ↓
              ChunkedBodyParser.feed()  ← YOUR CODE
              ↓
              writes decoded bytes to TempFile
```

I guess I need to asked what the entry point was, because I was looking at
`ChunkBodyParser.feed()` thoroughly but I forgot to ask myself where it was
called, so now it is in `RequestParser.feed() -> handleChunkedBody() -->
ChunkedBodyParser.feed() -> writes bytes TemFile`

question15: I should have think about the logic step as follows
```
```bash
Network (you know this)
      ↓
      ↓ raw bytes arrive
      ↓
  RequestParser.feed()     ← calls your code when chunked
      ↓
      ├─→ handleContentLengthBody()   ← not your problem
      └─→ handleChunkedBody()         ← YOUR ENTRY POINT
              ↓
              ChunkedBodyParser.feed()  ← YOUR CODE
              ↓
              writes decoded bytes to TempFile
```

After asking you for help, I went into details and then working it slowly.

My question would be are there are any strategies. I guess it is more like
Traffic Light system. before doing so, I would need to know what the big picture
is. the first thing I did was to read the link from the issue, and then
communicated with you, then I just fellow your suggestion, up to this point I
realised this flowchart provided a clear idea of navigating the project.

but also I learned that in 2025 I need to celebrate small wins and start small.
like actually this was the first time I sort of working on a large codebase in
C++98, so I gained some experience, made some mistakes, took notes (the file you
are reading) and asked a few questions, I believe I would get better next time.
keep working on the project, keep reading it, keep summarising, keep aksing
questions, I would get there.


---

## Answers (added 23:46, 2 hours of study)

### A1 (operator int() const)
This is a **conversion operator**. It lets you use a `TempFile` object wherever an `int` is expected.
```cpp
TempFile file;
file.open();
write(file, "hello", 5);  // works! file converts to int (the fd_)
```
Without it, you'd need: `write(file.fd(), "hello", 5)`. It's syntactic sugar.

### A2 (private copy constructor/assignment)
This is the **"non-copyable" idiom** (C++98 version). Your teammates did the RIGHT thing.
- A `TempFile` owns a file descriptor (a system resource)
- If you copy it, both objects think they own the same fd
- When one destructor runs, it closes the fd
- The other object now has an invalid fd → crash/bug

In C++11+ you'd write `= delete`. In C++98, making them private prevents copying.

### A3 (senior engineer navigation)
A senior would: (1) find the entry point first ("who calls this?"), (2) trace ONE path through the code, (3) ignore everything else until needed. Exactly what you started doing by the end of your notes.

### A4 (explicit keyword)
`explicit` prevents **implicit conversion**. Without it:
```cpp
void foo(ChunkedBodyParser p);
TempFile t;
foo(t);  // Would compile! Implicitly creates ChunkedBodyParser from TempFile
```
With `explicit`, you must write: `foo(ChunkedBodyParser(t))`. Prevents bugs.

And yes - `ChunkedBodyParser(utils::TempFile &file)` is the **constructor**. Same name as class = constructor.

### A5 (neovim/ctags)
ctags struggles with C++ overloaded functions. Better options:
- **clangd + LSP**: `gd` (go to definition) is context-aware
- **cscope**: Better for "who calls this function?"
- For now: use `grep -rn "ChunkedBodyParser::state"` to find the exact one

### A6 (mental models for C++98)
Key mental model: **"Everything belongs to something"**
- `RequestParser::State` = "State that belongs to RequestParser"
- `state_` with trailing underscore = "member variable of THIS class"
- `ClassName::functionName()` in .cpp = "implementation of that class's function"

### A7 (nature of C++98)
Yes, this is the nature of C++. The `ClassName::` prefix in .cpp files is required. You get used to it. The pattern to recognize: `ReturnType ClassName::methodName()` - always 3 parts.

### A8 (TODO comment meaning)
It means: "Right now this function does nothing. When you implement it, you'll need to:
- Change `state_` (the internal state machine)
- Write decoded bytes to `bodyFile_`"

### A9 (where ChunkedBodyParser fits)
You figured it out! It's part of REQUEST PARSING, before any handling happens.

### A10 (feeling lost)
Not a question, but your insight is correct: feeling overwhelmed = focusing on too much. Narrow scope.

### A11 (direction of thinking)
Your granular approach is GOOD. Following the trail (`buffer_` → where declared → what it does) is exactly right.

### A12a (terminology)
`ChunkedBodyParser` in `ChunkedBodyParser::State` is called the **"class name"** or **"scope"**. The `::` is the **"scope resolution operator"**. You'd say: "State is scoped to ChunkedBodyParser" or "ChunkedBodyParser's State enum".

### A12b (why you feel blocked) ⭐ MOST IMPORTANT
You're blocked because you understand the CODE but not the PROBLEM DOMAIN (chunked encoding format). You need to understand WHAT chunked data looks like before you can write code to parse it. See detailed answer below.

### A13 (states already designed)
The states are ALREADY designed for you in the header! You don't need to design them:
- `PARSING_CHUNK_SIZE` → reading the hex number
- `PARSING_CHUNK_DATA` → reading the actual bytes
- `PARSING_TRAILER` → reading the `\r\n` after data
- `DONE` → saw the `0\r\n\r\n` terminator

### A14 (thinking without assistant)
The key question you missed was: "What does the INPUT look like?" Before writing a parser, you must understand the format you're parsing. Always start with: "What am I transforming FROM and TO?"

### A15 (strategies)
Your Traffic Light analogy is good. The strategy is:
1. **Red**: What is the INPUT format? (chunked encoding spec)
2. **Yellow**: What is the OUTPUT? (decoded bytes to file)
3. **Green**: What are the STATES between input and output?

---

## ⭐ Detailed Answer for A12b (Your Real Blocker)

You can read all the code perfectly, but you still can't implement `feed()` because:

**You haven't internalized what chunked data LOOKS like.**

Here's the format - memorize this:
```
<size-in-hex>\r\n
<that-many-bytes>\r\n
<size-in-hex>\r\n
<that-many-bytes>\r\n
0\r\n
\r\n
```

Real example - sending "HelloWorld":
```
5\r\n
Hello\r\n
5\r\n
World\r\n
0\r\n
\r\n
```

Your state machine needs to:
1. `PARSING_CHUNK_SIZE`: Read until `\r\n`, convert hex to number (e.g., "5" → 5)
2. `PARSING_CHUNK_DATA`: Read exactly that many bytes, write to file
3. `PARSING_TRAILER`: Consume the `\r\n` after the data
4. Loop back to step 1, unless size was 0 → `DONE`

**Your next step**: Write this example on paper. Trace through it manually with the states. THEN the code will flow naturally.

