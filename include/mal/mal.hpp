#pragma comment(lib, "./include/curl/libcurl.lib")

#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <mal/jikan_cert.hpp>

#include <iostream>

constexpr const char* api_v = "v4"; /* API version. */

/* checks if the value is null. */
template<typename T> T is_null(nlohmann::json& j) noexcept {
	if (j.is_null()) return {};
	return j.get<T>();
}

enum image_size {
	s_normal, /* normal size via MAL website */
	s_small /* smaller than s_normal */,
	s_large /* larger than s_normal */
};

enum type {
	t_null, /* none mentioned */
	t_tv, /* anime series */
	t_special,
	t_ona, /* Original Net Animation */
	t_ova, /* Original Video Animation*/
	t_movie /* anime movie*/
};

class anime {
public:
	nlohmann::json j{}; /* get specific objects from the API */
	std::vector<std::string> image{}; /* the front image on MAL of that anime. enum: image_size */
	std::string en_title{}; /* English title */
	std::string jp_title{}; /* Japanese title */
	type type{}; /* anime type. e.g. t_tv = series, t_movie = movie. enum: type */
	short episodes{}; /* episodes within the anime. seasons wont effect the count, however some animes have multiple titles like AoT. */
	bool airing{}; /* still releasing episodes... */
	time_t released{}; /* the time when it released/aired. this uses time_t format */
	std::string duration{}; /* on average the length of each episode. */
	std::string rating{}; /* the level of maturity of this anime. e.g. R, PG */
	double score{}; /* medium of overall score of this anime. e.g. 1.0 - 10.0 */
	int scored_by{}; /* number of people who scored this anime. */
	int rank{}; /* anime ranking */
	int popularity{}; /* the overall popularity of this anime. */
	std::string synopsis{}; /* about the anime. */
	std::string background{}; /* about the production. */

	anime(const nlohmann::json& j) noexcept {
		this->j = j;
		if (is_null<std::string>(this->j["type"]) == "RateLimitException") rate_limited = true;
		else {
			for (const char* const& size : { "image_url", "small_image_url", "large_image_url" })
				this->image.push_back(is_null<std::string>(this->j["images"]["jpg"][size]));
			this->en_title = is_null<std::string>(this->j["title_english"]);
			this->jp_title = is_null<std::string>(this->j["title_japanese"]);
			this->type = (is_null<std::string>(this->j["type"]) == "Movie") ? type::t_movie :
				(is_null<std::string>(this->j["type"]) == "TV") ? type::t_tv :
				(is_null<std::string>(this->j["type"]) == "Special") ? type::t_special :
				(is_null<std::string>(this->j["type"]) == "ONA") ? type::t_ona :
				(is_null<std::string>(this->j["type"]) == "OVA") ? type::t_ova : type::t_null;
			this->episodes = is_null<short>(this->j["episodes"]);
			this->airing = is_null<bool>(this->j["airing"]);
			{
				std::unique_ptr<std::tm> tm = std::make_unique<std::tm>();
				tm->tm_year = is_null<int>(this->j["aired"]["prop"]["from"]["year"]);
				tm->tm_mon = is_null<int>(this->j["aired"]["prop"]["from"]["month"]) + 1;
				tm->tm_mday = is_null<int>(this->j["aired"]["prop"]["from"]["day"]);
				this->released = std::mktime(tm.get());
			} /* ~tm() */
			this->duration = is_null<std::string>(this->j["duration"]);
			this->rating = is_null<std::string>(this->j["rating"]);
			this->score = is_null<double>(this->j["score"]);
			this->scored_by = is_null<int>(this->j["scored_by"]);
			this->rank = is_null<int>(this->j["rank"]);
			this->popularity = is_null<int>(this->j["popularity"]);
			this->synopsis = is_null<std::string>(this->j["synopsis"]);
			this->background = is_null<std::string>(this->j["background"]);
		}
	}
	/* please take this to consideration as all anime information will be null if true.
	 * being aware of the rules is important! rate limit: 3req/sec
	 * @return bool true if rate limited
	*/
	bool is_ratelimit() const noexcept {
		return rate_limited;
	}
private:
	bool rate_limited{};
};

using anime_callback = std::function<void(const anime&)>;

/* for CURLOPT_WRITEFUNCTION. this was made by libcurl not me. */
size_t write_data(char* data, size_t size, size_t bytes, std::string* outcome) noexcept {
	outcome->append(std::move(data), size * bytes);
	return size * bytes;
}

/* replace string length data rather std::replace() replacing only char */
std::string replace(const std::string& str, char replace, std::string with) noexcept {
	std::string mstr{};
	for (const char& c : str)
		(c == replace) ? mstr += with : mstr += c;
	return std::move(mstr);
}

/* GET a specific anime via name
 * support spaces, mis-spelling. however this is not enhanced; wrong result is possible.
 * @param results the number of results. this is useful for getting a whole anime series collection like one piece movies, fate series, ect
 *   by default the value is 1 meaning it'll only GET the most relevant search based on the name.
 * @param callback used to get all results once function is done
*/
void anime_get(const std::string& name, const int& results, anime_callback callback) {
	std::unique_ptr<CURL, decltype(&curl_easy_cleanup)> curl(curl_easy_init(), curl_easy_cleanup);
	std::string all_data{};
	curl_easy_setopt(curl.get(), CURLOPT_URL, std::format("https://api.jikan.moe/{0}/anime?q=\"{1}\"&limit={2}",
		api_v, replace(name, ' ', "%20"), results).c_str());
	curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, write_data);
	curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, &all_data);
	if (not std::ifstream{ ".\\cacert.pem" }) {
		std::cout << "[MAL LOG] installing cacert.pem for accessing https://api.jikan.moe/" << std::endl;
		install_cert();
	}
	curl_easy_setopt(curl.get(), CURLOPT_CAINFO, ".\\cacert.pem");
	curl_easy_setopt(curl.get(), CURLOPT_CAPATH, ".\\cacert.pem");
	if (curl_easy_perform(curl.get()) == CURLE_OK) {
		nlohmann::json j = nlohmann::json::parse(std::move(all_data));
		for (const auto& data : j["data"])
			callback(std::move(anime(data)));
	}
}

/* being worked on... */
class manga {};

/* being worked on... */
class character {};

/* being worked on... */
class profile {};
