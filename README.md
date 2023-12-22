[![Codacy Badge](https://app.codacy.com/project/badge/Grade/a7a7adebae7b4d4e95f415c22b4227f5)](https://app.codacy.com/gh/LeeEndl/mal.cpp/dashboard?utm_source=gh&utm_medium=referral&utm_content=&utm_campaign=Badge_grade)

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

get website/API response time in seconds.
```C++
double palse = mal::api_palse();
...
```



> **NOTE**: this is blocked code- meaning following code will be paused until this function finishes.


<div/>
