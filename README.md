<div>

## including 
```C++
#include <mal/mal.hpp>
```

### basic usage
this function is used to GET anime/s then individually stored in `anime`. this ultimately makes it easier to access.
 
```C++
mal::anime_get("one piece", 5, [](const mal::anime& anime) {
	...
});
```

manga...
```C++
mal::manga_get("one piece", 5, [](const mal::manga& manga) {
	...
});
```


> **NOTE**: this is blocked code- meaning following code will be paused until this function finishes.


<div/>
