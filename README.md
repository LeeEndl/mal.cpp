<div>

## including 
```C++
#include <mal/mal.hpp>
```

### basic usage
understanding: it is used to GET all results mentioned in 2nd parameter then individually stored in `anime`. this ultimately makes easier to call
**NOTE**: this is blocked code- meaning following code will be paused until this function finishes
```C++
anime_get("one piece", 5, anime_callback([](const anime& anime) {
	if (anime.type == type::t_tv) {
		...
        }
}));
```

<div/>
