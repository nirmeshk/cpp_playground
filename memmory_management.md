### How do I deal with memory leaks?
source - https://www.stroustrup.com/bs_faq2.html#memory-leaks
ref - A brief introduction to C++’s model for type- and resource-safety by Bjarne Stroustrup, Herb Sutter & Gabriel Dos Reis 

By writing code that doesn't have any.
Clearly, if your code has new operations, delete operations, and pointer arithmetic all over the place, you are going to mess up somewhere and get leaks, stray pointers, etc. This is true independently of how conscientious you are with your allocations: eventually the complexity of the code will overcome the time and effort you can afford. It follows that successful techniques rely on hiding allocation and deallocation inside more manageable types. Good examples are the standard containers. They manage memory for their elements better than you could without disproportionate effort. Consider writing this without the help of string and vector:

```
	#include<vector>
	#include<string>
	#include<iostream>
	#include<algorithm>
	using namespace std;

	int main()	// small program messing around with strings
	{
		cout << "enter some whitespace-separated words:\n";
		vector<string> v;
		string s;
		while (cin>>s) v.push_back(s);

		sort(v.begin(),v.end());

		string cat;
		typedef vector<string>::const_iterator Iter;
		for (Iter p = v.begin(); p!=v.end(); ++p) cat += *p+"+";
		cout << cat << '\n';
	}
```

What would be your chance of getting it right the first time? And how would you know you didn't have a leak?
Note the absence of explicit memory management, macros, casts, overflow checks, explicit size limits, and pointers. By using a function object and a standard algorithm, I could have eliminated the pointer-like use of the iterator, but that seemed overkill for such a tiny program.

These techniques are not perfect and it is not always easy to use them systematically. However, they apply surprisingly widely and by reducing the number of explicit allocations and deallocations you make the remaining examples much easier to keep track of. As early as 1981, I pointed out that by reducing the number of objects that I had to keep track of explicitly from many tens of thousands to a few dozens, I had reduced the intellectual effort needed to get the program right from a Herculean task to something manageable, or even easy.

If your application area doesn't have libraries that make programming that minimizes explicit memory management easy, then the fastest way of getting your program complete and correct might be to first build such a library.

Templates and the standard libraries make this use of containers, resource handles, etc., much easier than it was even a few years ago. The use of exceptions makes it close to essential.

If you cannot handle allocation/deallocation implicitly as part of an object you need in your application anyway, you can use a resource handle to minimize the chance of a leak. Here is an example where I need to return an object allocated on the free store from a function. This is an opportunity to forget to delete that object. After all, we cannot tell just looking at pointer whether it needs to be deallocated and if so who is responsible for that. Using a resource handle, here the standard library auto_ptr, makes it clear where the responsibility lies:
```
	#include<memory>
	#include<iostream>
	using namespace std;

	struct S {
		S() { cout << "make an S\n"; }
		~S() { cout << "destroy an S\n"; }
		S(const S&) { cout << "copy initialize an S\n"; }
		S& operator=(const S&) { cout << "copy assign an S\n"; }
	};

	S* f()
	{
		return new S;	// who is responsible for deleting this S?
	};

	auto_ptr<S> g()
	{
		return auto_ptr<S>(new S);	// explicitly transfer responsibility for deleting this S
	}

	int main()
	{
		cout << "start main\n";
		S* p = f();
		cout << "after f() before g()\n";
	//	S* q = g();	// this error would be caught by the compiler
		auto_ptr<S> q = g();
		cout << "exit main\n";
		// leaks *p
		// implicitly deletes *q
	}
    ```

Think about resources in general, rather than simply about memory.

If systematic application of these techniques is not possible in your environment (you have to use code from elsewhere, part of your program was written by Neanderthals, etc.), be sure to use a memory leak detector as part of your standard development procedure, or plug in a garbage collector.