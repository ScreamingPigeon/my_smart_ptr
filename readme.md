# README

## My implementation of CPP smart pointers (originally introduced for C++03 in 2007)

I know nearly nothing about CPP, and object oriented programming. I like procedural C code
(bonus points if it is reentrant).

The rationale for this project comes from a desire to better understand this language, and
the paradigms of modern object oriented programming (and to figure out smart pointers i guess).

This markdown file serializes my thought process, where almost
all observations appear in chronological order. There are a couple modifications
made for the purpose of improved readability and organization, but this should
accurately reflect my learning process.

## What are smart pointers?

> to give ownership of any heap-allocated resource—for example,
dynamically-allocated memory or system object handles—to a stack-allocated
object whose destructor contains the code to delete or free the resource and
also any associated cleanup code.

[source](https://learn.microsoft.com/en-us/cpp/cpp/smart-pointers-modern-cpp?view=msvc-170)


## Process

I am going to read a few resources (listed below) on smart pointers. I shall attempt only look
at the the library's signature and attempt to implement everything else on my own. I will then
compare my final product with the existing library implementation at the end. For a realistic
timeline, i will only be implementing 

- `unique_ptr`
- `shared_ptr`
- `weak_ptr`

I will write helper functions and classes as needed.

Resources:
- https://learn.microsoft.com/en-us/cpp/cpp/smart-pointers-modern-cpp?view=msvc-170
-

Begin.



## Initial Notes

Big-picture details:
- Implemented to wrap around regular pointers to fix:
    - mem leaks
    - buffer overflows
    - invalidated addresses
- Wraps around regular pointers with
    - operator overloading on `*` and `->` ()
    - destructor (called automatically when object goes out of scope)
    
Technical details:
- Defined in `<memory>` header 


### Thoughts as I proceed

1. Defining a class structure
Ok. So now I have an idea of what to do. Let's start by setting up a project dir
```
$> mkdir my_smart_ptr
$> cd my_smart_ptr
$> mkdir build
$> mkdir src
```


* Now What?*


Time to define a header file that can be imported. I decide to go with `memory.hpp` to try
to stay consistent with the original class being imported from `<memory>`.


* Now?*


Ok. Class declaration/signatures. I vaguely know what inheritance is. Do i need it here? Do
the `unique_ptr`, `shared_ptr`, and `weak_ptr` classes have anything in common. Time to look 
at what (cppreference.com)[http://cppreference.com] has to say ( ! and         are words I use to indicate the presence of 
a member in one of the other columns).


|                  | `unique_ptr` | `shared_ptr` | `weak_ptr` |
|------------------|--------------|--------------|------------|
| Member Functions | constructor  |              |            | 
|                  | destructor   |              |            |
|                  | operator=    |              |            |
|                  |              |              |            |
| Modifiers        | reset        |              |            |
|                  | swap         |              |            |
|                  | release      |      !       |     !      |
|                  |              |              |            |
| Observers        | get          |              |     !      |
|                  | get_deleter  |      !       |     !      |
|                  | operator bool|              |     !      |
|                  | operator*    |              |     !      |
|                  | operator->   |              |     !      |
|                  | operator[]   |              |     !      |
|                  |     !        | owner_before |            |
|                  |     !        | owner_hash   |            |
|                  |     !        | owner_equal  |            |
|                  |     !        | use_count    |            |
|                  |     !        |      !       | expired    |
|                  |     !        |      !       | lock       |


Lot of overlaps. Maybe we could benefit from using some sort of inheritance. How do 
I setup such a class scheme? 

After thinking about this some more (and some online searches) - I realized that my
implementation needs to be able to "point" for all types (`int`,
`char`, `std::vector<int>`). So setting up a *template* for the class would be
beneficial. For the sake of simplicity, i am going to start with just the primitive types
first


*What is a template?*

> When template arguments are provided, or, for function and
class(since C++17) templates only, deduced, they are substituted 
for the template parameters to obtain a specialization of the template,
that is, a specific type or a specific function lvalue. 

In other words, you can write classes that take a template argument (in our case)
the kind of pointer, and then have the class wrap around those types. There seem
to be non-type templates and template templates but I'm not gonna get into those.

I would assume we need to enforce some sort of constraint that our template
can only work with pointers, but I'm not sure how to get that to work at the moment.
If you do - please feel free to email me or leave an issue / PR somewhere on the github.


Let's start by making a base `smart_ptr` class first.


```
template <typename T> class smart_ptr {
    public:
        T *data;
        // Constructor
        smart_ptr();

        // Destructor
        ~smart_ptr();

        // Operator Overloading
        bool operator=(smart_ptr other_ptr);

        // Modifiers
        void reset();
        void swap();
};

template <typename T> class unique_ptr : smart_ptr<T> {
        unique_ptr release();
        T get();
        void *get_deleter(); // Strange
        explicit operator bool() const noexcept; // Strange
        T *operator*();
        T *operator->();
        T *operator[](int i); 
};

template <typename T> class shared_ptr : smart_ptr<T> {
        T get();
        explicit operator bool();
        T *operator*();
        T *operator->();
        T *operator[](int i); 
        T owner_before();
        T owner_hash();
        T owner_equal();
        int use_count();
};

template <typename T> class weak_ptr : smart_ptr<T> {
        T get();
        T owner_before();
        T owner_hash();
        T owner_equal();
        int use_count();
        int expired();
        int lock();
};
```
footer: this code was written on 08/13/2024

Note: I did have to pause for a moment to figure out how to overload
the = operator. I had setup something like `operator=();` intially
and used LSP's (I use [`clangd`](https://clangd.llvm.org/installation.html)) guidance to come up with the rest of the syntax for the overloading.

I plan on implementing helper functions as and when needed.

#### Thoughts

##### Templates
Standard C++ smart pointers work with the following templates.

```
template<

    class T,

    class Deleter = std::default_delete<T>

> class unique_ptr;
 
------

template <

    class T,

    class Deleter

> class unique_ptr<T[], Deleter>;
```

Where `deleter` is used in the destructor, and can be set to a custom class.
For the sake of simplicity,I will simply `delete` the heap-allocated object,
and avoid the secondary parameter in the constructor.

##### Keywords
I encountered the `explicit` keyword and started wondering about the meaning
of this and many other keywords in C++. An exhaustive list can be found 
[here](https://en.cppreference.com/w/cpp/keyword). 
I found a nice stack overflow thread that explains what
[`explicit`](https://stackoverflow.com/questions/121162/what-does-the-explicit-keyword-mean)
does.

##### Strange functions
You may notice, that my class declarations have a couple member functions that have
a comment indicating that they are somehow "strange". This is mostly because I could
not figure out what these functions did at first glance 😂.

After some extra reading, here is more information on the functions
- `void *get_deleter();` -  (`unique_ptr`): It seems to return a pointer to the
custom deleter passed into the template while creating a smart pointer. Since we
are not supporting custom deleters at the moment, i can skip this from my declaration.
- `explicit operator bool() const noexcept` - (`unique_ptr`): Well this is supposed to check if
the stored pointer is not null. While this sounds simple enough, my unfamaliarity
with overloading and the explicit keyword make this appear quite confusing.

All this overloading does is define the behavior of a smart pointer when used in 
boolean operations. The `explicit` stops the compiler from doing wonky things 
(I don't fully understand how though). The `operator bool()` defines the type
conversion. The `const` implies that no changes are made to the state of the 
object. `noexcept` states that this overloading doesn't throw errors.


##### Inheritance and `virtual`
I wanted to make a quick note on Inheritance and Virtual classes/functions.


## Writing code

