<div>

## including 
```C++
#include <mal/mal.hpp>
```

### basic usage
this function is used to GET anime/s then individually stored in `anime`. this ultimately makes it easier to access.
 
```C++
anime_get("one piece", 5, anime_callback([](const anime& anime) {
	...
}));
```
**NOTE**: this is blocked code- meaning following code will be paused until this function finishes.


getting the anime title(s).
```C++
anime_get("one piece", 5, anime_callback([](const anime& anime) {
	std::cout << "english title: " << anime.en_title << " japanese title: " << anime.jp_title;
}));
```

Manually navigating anime information using JSON
```C++
anime_get("one piece", 5, anime_callback([](const anime& anime) {
	std::cout << "this anime came out in: " << j["year"];
}));
```
**NOTE**: used as a last resort. please consider `anime` classes variety of information offered.

<div/>
