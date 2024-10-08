## About me

This repository is to learn c++ by working through some examples.
Since most of the books and tutorials are catered towards new developers, this will act as a reference of code samples for the things that i use frequently. Bonus will be to implement some non trivial example in cpp.

## Build and Run
```
cd build
cmake ../
cmake --build .
```

Run:

```
./hello

## o/p

Hello World Nirmesh!
Heyo
```

Run Tests:

```
cmake --build . --target run_tests
```

Run Test from Vscode

- Make sure you have installed [CMake Test Explorer](https://marketplace.visualstudio.com/items?itemName=fredericbonnet.cmake-test-adapter)
- Go to Test tab from the left side of the panel in vscode, you should see the tests already discovered ![image](images/Screenshot2024-09-03-9.19.55 PM.png)
- Run individual tests Or all tests together

## Automated Formatting with clang

- install clang `brew install llvm`
- make sure to add the path to .zshrc (`echo 'export PATH="/opt/homebrew/opt/llvm/bin:$PATH"' >> ~/.zshrc`)
- `source ~/.zshrc`
- Now any default build should trigger the clang format
- To call the format target explicitly, call `cmake --build . --target format`

## IDE Setup
- After brief seraching around, looks like VS Code is the choice of editor if you are getting started. Clion has good review, but is not free. Visual studio for C++ is more advanced, but only available for windows :(. So for now, VSCode it is.
- Setup vs code using instructions here -> https://code.visualstudio.com/docs/cpp/config-clang-mac
- There was a hiccup where `xcode-select --install` , the `clang --version` was throwing an error. This was because the install command returns immediately, while there was a background window which was still finishing up the installation. Rest of the setup was strainght forward.
- I also added `"${fileDirname}/build/${fileBasenameNoExtension}"` in the args in tasks.json in order to alwasy create the executables inside the build/ folder that can be easily added to `.gitignore`. TODO: might have to revisit this as the structure of the program evolves and I start introducing tests and folders.
- Once the VScode is isntalled,
  + install https://marketplace.visualstudio.com/items?itemName=matepek.vscode-catch2-test-adapter to run tests
  + install https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools to get Vscode working well with cpp
- Install CPP check plugin. You will also need to install cppcheck (`brew install Cppcheck`)

## TODOs
- [x] hello world program
- [x] hello world with cmake
- [x] Running automated tests with cmake
- [x] Running automated formatting with cmake
- [x] Use of std::map (ordered), std::unordered_map
- [x] Use of array list equivalent (vector)
- [ ] Two dimensional array (with rows of different sizes)
- [ ] use of sets
- [ ] use of for/while loop
- [ ] use of exceptions (try/catch??)
- [ ] basic use of classes and objects
- [ ] implement DFS
- [ ] implement BFS
- [ ] implement templated/concurrent trie https://15445.courses.cs.cmu.edu/fall2022/project0/
- [ ] (Stretch goal) implement matrix multiplication from previous course https://github.com/cmu-db/bustub/commit/bf5e92aa6a20d659b0df4c3de5f6a9a56bf4edc8
- [ ] implament OR-Set
- [ ] implement merging intervals
- [ ] implement a toy embedded sqlite like database

## Quirky Topics
- [] [Examples of Usage of Move](src/move_example_1.cpp)
- [] [Perf impact of move](src/move_example_2.cpp)
- [] [Performance impact of pointer in-direction](src/pointer_indirection.cpp)
- [] [(WIP) Impact of memory allocation](src/memory_allocation_vs_pooling.cpp)

## Suplimentary readings
- [Some notes about how c++ memmory management works](memmory_management.md)
- https://www.programiz.com/cpp-programming/variables-literals  (Quality tutorial)
- https://www.geeksforgeeks.org/header-files-in-c-cpp-and-its-uses/
- https://github.com/rigtorp/awesome-modern-cpp
- https://www.reddit.com/r/cpp/comments/b3115b/beautiful_c_codebases/
- https://herbsutter.com/
- https://www.thecodedmessage.com/posts/cpp-move/

## Interesting projects
- https://github.com/google/leveldb
- https://github.com/scylladb/seastar
- https://github.com/facebook/folly
