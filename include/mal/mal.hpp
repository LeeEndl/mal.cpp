#pragma comment(lib, "./include/curl/libcurl.lib")

#include <curl/curl.h>
#include <nlohmann/json.hpp>

#ifdef mal_DEBUG
#include <fstream>
#include <iostream>
#endif

constexpr const char* api_v = "v4"; /* API version. this is not static! */

/* made for libcurl CURLOPT_WRITEFUNCTION. this was made by libcurl not me. */
size_t write_data(char* data, size_t size, size_t bytes, std::string* outcome) noexcept {
	outcome->append(std::move(data), size * bytes);
	return size * bytes;
}

/* replace string length data rather std::replace() replacing only char */
std::string replace(std::string str, char replace, std::string with) noexcept {
	std::string mstr{};
	for (char c : str)
		(c == replace) ? mstr += with : mstr += c;
	return std::move(mstr);
}

/*
* GET a specfic anime via ID and store it inside JSON format.
* @param id the anime id via https://myanimelist/anime/
*/
nlohmann::json anime_get_raw(std::string name) noexcept {
	std::string fname{ replace(std::move(name), ' ', "%20") };
	std::unique_ptr<CURL, decltype(&curl_easy_cleanup)> curl(curl_easy_init(), curl_easy_cleanup);
	std::string data{};
	curl_easy_setopt(curl.get(), CURLOPT_URL, std::format("https://api.jikan.moe/{0}/anime?q=\"{1}\"", api_v, fname).c_str());
	curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, write_data);
	curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, &data);
#ifdef mal_DEBUG
	if (not std::ifstream{ ".\\cacert.pem" })
		std::cout << "[MAL LOG] could not locate cacert.pem" << std::endl;
#else
		curl_easy_setopt(curl.get(), CURLOPT_CAINFO, ".\\cacert.pem");
		curl_easy_setopt(curl.get(), CURLOPT_CAPATH, ".\\cacert.pem");
#endif
	if (curl_easy_perform(curl.get()) == CURLE_OK) return nlohmann::json::parse(std::move(data));
	return {};
}

/* checks if the value is null. idk if JSON already has this feature... lol */
template<typename T> T is_null(nlohmann::json j) noexcept {
	if (j.is_null()) return {};
	return j.get<T>();
}

enum image_size { s_normal, s_small, s_large };

class anime {
public:
	std::vector<std::string> image{}; /* the front image on MAL of that anime. enum: image_size */
	std::string title{}; /* anime title. */
	short episodes{}; /* episodes within the anime. seasons wont effect the count, however some animes have multiple titles like AoT. */
	bool airing{}; /* still releasing episodes... */
	time_t released{}; /* the time when it released/aired. this uses time_t format */
	std::string duration{}; /* on average the length of each episode. */
	double score{}; /* medium of overall score of this anime. 1.0 - 10.0 */
	int scored_by{}; /* number of people who scored this anime. */
	int popularity{}; /* the overall popularity of this anime. */
	std::string synopsis{}; /* about the anime. */
	std::string background{}; /* about the production. */

	anime(std::string name) noexcept {
		this->j = anime_get_raw(std::move(name));
		if (not j["data"].empty()) {
			this->image.push_back(is_null<std::string>(this->j["data"][0]["images"]["jpg"]["image_url"]));
			this->image.push_back(is_null<std::string>(this->j["data"][0]["images"]["jpg"]["small_image_url"]));
			this->image.push_back(is_null<std::string>(this->j["data"][0]["images"]["jpg"]["large_image_url"]));
			this->title = is_null<std::string>(this->j["data"][0]["title"]);
			this->episodes = is_null<short>(this->j["data"][0]["episodes"]);
			this->airing = is_null<bool>(this->j["data"][0]["airing"]);
			{
				std::unique_ptr<std::tm> tm = std::make_unique<std::tm>();
				tm->tm_year = is_null<int>(this->j["data"][0]["aired"]["prop"]["from"]["year"]);
				tm->tm_mon = is_null<int>(this->j["data"][0]["aired"]["prop"]["from"]["month"]) + 1;
				tm->tm_mday = is_null<int>(this->j["data"][0]["aired"]["prop"]["from"]["day"]);
				this->released = std::mktime(tm.get());
			}
			this->duration = is_null<std::string>(this->j["data"][0]["duration"]);
			this->score = is_null<double>(this->j["data"][0]["score"]);
			this->scored_by = is_null<int>(this->j["data"][0]["scored_by"]);
			this->popularity = is_null<int>(this->j["data"][0]["popularity"]);
			this->synopsis = is_null<std::string>(this->j["data"][0]["synopsis"]);
			this->background = is_null<std::string>(this->j["data"][0]["background"]);
		}
	}
private:
	nlohmann::json j{};
};

/* being worked on... */
class manga { private: };

/* being worked on... */
class character { private: };

/* being worked on... */
class profile {private: };

