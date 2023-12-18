#pragma comment(lib, "./include/curl/libcurl.lib")

#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <mal/jikan_cert.hpp>

constexpr size_t write_data(char* data, size_t size, size_t bytes, std::string* outcome) noexcept {
	outcome->append(std::move(data), size * bytes);
	return size * bytes;
}

std::string replace(const std::string& str, char replace, std::string with) noexcept {
	std::stringstream mstr{};
	for (const char& c : str)
		(c == replace) ? mstr << with : mstr << c;
	return std::move(mstr.str());
}

template<typename T> T is_null(const nlohmann::json& j) noexcept {
	if (j.is_null()) return {};
	return j.get<T>();
}

void request(std::function<void(std::unique_ptr<CURL, decltype(&curl_easy_cleanup)>&)> request) noexcept {
	std::unique_ptr<CURL, decltype(&curl_easy_cleanup)> curl{ curl_easy_init(), curl_easy_cleanup };
	if (not std::ifstream{ ".\\cacert.pem" })
		std::ofstream{ ".\\cacert.pem" } << jikan_cert;
	curl_easy_setopt(curl.get(), CURLOPT_CAINFO, ".\\cacert.pem");
	curl_easy_setopt(curl.get(), CURLOPT_CAPATH, ".\\cacert.pem");
	curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, write_data);
	request(curl);
}

namespace mal {
	enum image_size {
		s_normal, /* normal size via MAL website */
		s_small /* smaller than s_normal */,
		s_large /* larger than s_normal */
	};

	/* manga is static. */
	enum type {
		t_null, /* none mentioned */
		t_tv, /* anime series */
		t_special, /* anime special */
		t_ona, /* Original Net Animation */
		t_ova, /* Original Video Animation*/
		t_movie /* anime movie */
	};

	class anime {
	public:
		int mal_id{};
		std::vector<std::string> image{}; /* the front image on MAL of that anime. enum: image_size */
		std::string title{}; /* recommended title. as some animes only have japanese title. */
		std::string jp_title{}; /* Japanese title. */
		type type{}; /* anime type. e.g. t_tv = series, t_movie = movie. enum: type */
		short episodes{}; /* episodes within the anime. seasons wont effect the count, however some animes have multiple titles like AoT. */
		bool airing{}; /* still releasing episodes... */
		std::tm released{}; /* the time when it released/aired.  */
		std::tm next_release{}; /* the time when the next episode releases.   */
		std::string duration{}; /* on average the length of each episode. */
		std::string rating{}; /* the level of maturity of this anime. e.g. R, PG */
		double score{}; /* medium of overall score of this anime. e.g. 1.0 - 10.0 */
		int scored_by{}; /* number of people who scored this anime. */
		int rank{}; /* anime ranking. */
		int popularity{}; /* the overall popularity of this anime. */
		int favorites{}; /* total favorites. */
		std::string synopsis{}; /* about the anime. */
		std::string background{}; /* about the production. */

		anime(const nlohmann::json& j) noexcept {
			this->mal_id = is_null<int>(j["mal_id"]);
			for (const char* const& size : { "image_url", "small_image_url", "large_image_url" })
				this->image.push_back(is_null<std::string>(j["images"]["jpg"][size]));
			this->title = is_null<std::string>(j["title"]);
			this->jp_title = is_null<std::string>(j["title_japanese"]);
			this->type = (is_null<std::string>(j["type"]) == "Movie") ? type::t_movie :
				(is_null<std::string>(j["type"]) == "TV") ? type::t_tv :
				(is_null<std::string>(j["type"]) == "Special") ? type::t_special :
				(is_null<std::string>(j["type"]) == "ONA") ? type::t_ona :
				(is_null<std::string>(j["type"]) == "OVA") ? type::t_ova : type::t_null;
			this->episodes = is_null<short>(j["episodes"]);
			this->airing = is_null<bool>(j["airing"]);
			this->released.tm_year = is_null<int>(j["aired"]["prop"]["from"]["year"]);
			this->released.tm_mon = is_null<int>(j["aired"]["prop"]["from"]["month"]) + 1;
			this->released.tm_mday = is_null<int>(j["aired"]["prop"]["from"]["day"]);
			this->next_release.tm_year = is_null<int>(j["aired"]["prop"]["to"]["year"]);
			this->next_release.tm_mon = is_null<int>(j["aired"]["prop"]["to"]["month"]) + 1;
			this->next_release.tm_mday = is_null<int>(j["aired"]["prop"]["to"]["day"]);
			this->duration = is_null<std::string>(j["duration"]);
			this->rating = is_null<std::string>(j["rating"]);
			this->score = is_null<double>(j["score"]);
			this->scored_by = is_null<int>(j["scored_by"]);
			this->rank = is_null<int>(j["rank"]);
			this->popularity = is_null<int>(j["popularity"]);
			this->favorites = is_null<int>(j["favorites"]);
			this->synopsis = is_null<std::string>(j["synopsis"]);
			this->background = is_null<std::string>(j["background"]);
		}
		constexpr ~anime() = default;
	private:
	};

	/* GET a specific anime via name
	 * support spaces, mis-spelling. however this is not enhanced; wrong result is possible.
	 * @param name this can be any string type, long as it supports std::format()
	 * @param results the number of results. this is useful for getting a whole anime series collection like one piece movies, fate series, ect
	 * @param callback used to get all results once function is done
	*/
	template<typename T>
	void anime_get(const T& name, const short& results, std::function<void(const anime&)> callback) noexcept {
		if (results > SHRT_MAX) return;
		request([&name, &results, &callback](std::unique_ptr<CURL, decltype(&curl_easy_cleanup)>& curl) {
			std::unique_ptr<std::string> all_data{ new std::string };
			std::unique_ptr<nlohmann::json> j{ new nlohmann::json };
			for (short page = 1; page <= (results + 24) / 25; ++page) {
				curl_easy_setopt(curl.get(), CURLOPT_URL, std::format("https://api.jikan.moe/v4/anime?q=\"{0}\"&limit={1}&page={2}",
					replace(name, ' ', "%20"), (results < 25) ? results : 25, page).c_str());
				curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, all_data.get());
				if (curl_easy_perform(curl.get()) not_eq CURLE_OK) break;
				*j = nlohmann::json(nlohmann::json::parse(*all_data));
				all_data->clear();
				if (is_null<int>((*j)["pagination"]["items"]["count"]) == 0) break;
				for (const nlohmann::json& data : (*j)["data"])
					callback(std::move(anime(data)));
			}
			});
	}

	class manga {
	public:
		int mal_id{};
		std::vector<std::string> image{}; /* the front image on MAL of that manga. enum: image_size */
		std::string title{}; /* recommended title. as some manga only have japanese title. */
		std::string jp_title{}; /* Japanese title. */
		int chapters{}; /* amount of chapters this manga contains. */
		int volumes{}; /* amount of volumes this manga contains. */
		bool publishing{}; /* if the manga is still being published. */
		std::tm published{}; /* when the manga was published */
		std::tm next_publish{}; /* when the manga's next published chapter */
		double score{}; /* medium of overall score of this manga. e.g. 1.0 - 10.0 */
		int scored_by{}; /* number of people who scored this manga. */
		int rank{}; /* manga ranking. */
		int popularity{}; /* the overall popularity of this manga. */
		int favorites{}; /* total favorites. */
		std::string synopsis{}; /* about the manga. */
		std::string background{}; /* about the production. */
		manga(const nlohmann::json& j) noexcept {
			this->mal_id = is_null<int>(j["mal_id"]);
			for (const char* const& size : { "image_url", "small_image_url", "large_image_url" })
				this->image.push_back(is_null<std::string>(j["images"]["jpg"][size]));
			this->title = is_null<std::string>(j["title"]);
			this->jp_title = is_null<std::string>(j["title_japanese"]);
			this->chapters = is_null<int>(j["chapters"]);
			this->volumes = is_null<int>(j["volumes"]);
			this->publishing = is_null<bool>(j["publishing"]);
			this->published.tm_year = is_null<int>(j["published"]["prop"]["from"]["year"]);
			this->published.tm_mon = is_null<int>(j["published"]["prop"]["from"]["month"]) + 1;
			this->published.tm_mday = is_null<int>(j["published"]["prop"]["from"]["day"]);
			this->next_publish.tm_year = is_null<int>(j["published"]["prop"]["to"]["year"]);
			this->next_publish.tm_mon = is_null<int>(j["published"]["prop"]["to"]["month"]) + 1;
			this->next_publish.tm_mday = is_null<int>(j["published"]["prop"]["to"]["day"]);
			this->score = is_null<double>(j["score"]);
			this->scored_by = is_null<int>(j["scored_by"]);
			this->rank = is_null<int>(j["rank"]);
			this->popularity = is_null<int>(j["popularity"]);
			this->favorites = is_null<int>(j["favorites"]);
			this->synopsis = is_null<std::string>(j["synopsis"]);
			this->background = is_null<std::string>(j["background"]);
		}
		constexpr ~manga() = default;
	};

	/* GET a specific manga via name
	 * support spaces, mis-spelling. however this is not enhanced; wrong result is possible.
	 * @param name this can be any string type, long as it supports std::format()
	 * @param results the number of results. this is useful for getting a whole manga collection
	 * @param callback used to get all results once function is done
	*/
	template<typename T>
	void manga_get(const T& name, const short& results, std::function<void(const manga&)> callback) noexcept {
		if (results > SHRT_MAX) return;
		request([&name, &results, &callback](std::unique_ptr<CURL, decltype(&curl_easy_cleanup)>& curl) {
			std::unique_ptr<std::string> all_data{ new std::string };
			std::unique_ptr<nlohmann::json> j{ new nlohmann::json };
			for (short page = 1; page <= (results + 24) / 25; ++page) {
				curl_easy_setopt(curl.get(), CURLOPT_URL, std::format("https://api.jikan.moe/v4/manga?q=\"{0}\"&limit={1}&page={2}",
					replace(name, ' ', "%20"), (results < 25) ? results : 25, page).c_str());
				curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, all_data.get());
				if (curl_easy_perform(curl.get()) not_eq CURLE_OK) break;
				*j = nlohmann::json(nlohmann::json::parse(*all_data));
				all_data->clear();
				if (is_null<int>((*j)["pagination"]["items"]["count"]) == 0) break;
				for (const nlohmann::json& data : (*j)["data"])
					callback(std::move(manga(data)));
			}
			});
	}
}
