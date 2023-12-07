<div>

### including 
```C++
#include <mal/mal.hpp>
```

### declaration
```C++
auto ani_it = anime_get_raw("one piece", 2);
```

### usage (e.g. filter out movies)
```C++
#include <iostream>

...

for (const auto& info : ani_it)
{
	  anime ani(info);
	  if (ani.type == type::t_tv)
		  std::cout << ani.en_title << std::endl;
}
```

<div/>
