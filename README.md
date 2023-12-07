<div>

## including 
```C++
#include <mal/mal.hpp>
```

# Basic Tutorial(s): 

#### get only tv series
```C++
anime_get("one piece", 5, anime_callback([](const anime& anime) {
	if (anime.type == type::t_tv) {
		...
        }
}));
```

<div/>
