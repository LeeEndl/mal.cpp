<div>

## including 
```C++
#include <mal/mal.hpp>
```

### basic usage
this function is used to GET anime/s then individually stored in `anime`. this ultimately makes it easier to access.
 
```C++
mal::anime_get("one piece", 5, [](mal::smart_anime anime) {
	...
});
```
**NOTE**: this is blocked code- meaning following code will be paused until this function finishes.


getting the anime title(s).
```C++
mal::anime_get("one piece", 5, [](mal::smart_anime anime) {
	std::cout << "english title: " << anime->title << " japanese title: " << anime->jp_title;
});
```

<div/>
