<div>

## including 
```C++
#include <mal/mal.hpp>
```

### basic usage
this function is used to GET anime/s then individually stored in `anime`. this ultimately makes it easier to access.
 
```C++
mal::search<mal::anime>("one piece", 5, [](auto anime) {
	...
});
```

manga...
```C++
mal::search<mal::manga>("one piece", 5, [](auto manga) {
	...
});
```


> **NOTE**: this is blocked code- meaning following code will be paused until this function finishes.


<div/>
