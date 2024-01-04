#pragma comment(lib, "./include/openssl/libcrypto.lib")
#pragma comment(lib, "./include/openssl/libssl.lib")

#include <openssl/ssl.h>
#include <openssl/bio.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace mal_dev {
	std::string replace(const std::string& str, char replace, const std::string& with) noexcept {
		std::stringstream mstr{};
		for (const char& c : str)
			(c == replace) ? mstr << with : mstr << c;
		return std::move(mstr.str());
	}

	template<typename T> T is_null(const nlohmann::json& j) noexcept {
		if (j.is_null()) return {};
		return j.get<T>();
	}

	using bio_callback = std::unique_ptr<BIO, decltype(&::BIO_free_all)>&;

	void request(std::function<void(bio_callback)> callback) {
		std::unique_ptr<SSL_CTX, decltype(&::SSL_CTX_free)> ctx{ SSL_CTX_new(TLS_client_method()), ::SSL_CTX_free };
		std::unique_ptr<BIO, decltype(&::BIO_free_all)> bio{ BIO_new_ssl_connect(ctx.get()), ::BIO_free_all };
		ssl_st* ssl{};
		BIO_ctrl(bio.get(), BIO_C_GET_SSL, 0L, reinterpret_cast<char*>(&ssl));
		SSL_ctrl(ssl, SSL_CTRL_MODE, SSL_MODE_AUTO_RETRY, nullptr);
		BIO_ctrl(bio.get(), BIO_C_SET_CONNECT, 0L, const_cast<char*>("api.jikan.moe:https"));
		callback(bio);
	}
}

#pragma once
using namespace mal_dev;

namespace mal {
	enum image_size {
		s_normal, /* normal size via MAL website */
		s_small /* smaller than s_normal */,
		s_large /* larger than s_normal */
	};

	enum type {
		t_null,
		t_tv, /* anime series */
		t_special, /* anime special */
		t_ona, /* Original Net Animation */
		t_ova, /* Original Video Animation*/
		t_movie /* anime movie */
	};

	class anime {
	public:
		short mal_id{};
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
		short scored_by{}; /* number of people who scored this anime. */
		short rank{}; /* anime ranking. */
		short popularity{}; /* the overall popularity of this anime. */
		short favorites{}; /* total favorites. */
		std::string synopsis{}; /* about the anime. */
		std::string background{}; /* about the production. */

		explicit anime(const json& j) noexcept {
			this->mal_id = is_null<short>(j["mal_id"]);
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
			this->scored_by = is_null<short>(j["scored_by"]);
			this->rank = is_null<short>(j["rank"]);
			this->popularity = is_null<short>(j["popularity"]);
			this->favorites = is_null<short>(j["favorites"]);
			this->synopsis = is_null<std::string>(j["synopsis"]);
			this->background = is_null<std::string>(j["background"]);
		}
		constexpr ~anime() = default;
	private:
	};

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
		explicit manga(const json& j) noexcept {
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

	/* API response time. via /v4/...
	 * @return response time in seconds
	*/
	double api_palse() {
		std::chrono::steady_clock::time_point end{};
		std::chrono::steady_clock::time_point start = std::chrono::high_resolution_clock::now();
		request([&start, &end](bio_callback bio) {
			BIO_puts(bio.get(), "GET /v4/ HTTP/1.1\r\nHost: api.jikan.moe\r\nConnection: Close\r\n\r\n");
			end = std::chrono::high_resolution_clock::now();
			});
		return std::chrono::duration<double>(end - start).count();
	}

	/* a basic GET search on http://myanimelist.net/
	 * support spaces, mis-spelling. however this is not enhanced; wrong result is possible.
	 * @param name this can be any string type, long as it supports std::format()
	 * @param results the number of results. this is useful for getting a whole series collection
	 * @param callback used to get all results once function is done
	*/
	template<typename T, typename string_Type>
	void search(const string_Type& name, const short& results, std::function<void(const T&)> callback) {
		if (results > 32767) return;
		request([&](bio_callback bio) {
			for (short page = 1; page <= (results + 24) / 25; ++page) {
				BIO_puts(bio.get(),
					std::format("GET /v4/{0}?q=\"{1}\"&limit={2}&page={3} HTTP/1.1\r\nHost: api.jikan.moe\r\nConnection: Close\r\n\r\n",
						std::string(typeid(T).name()).substr(sizeof("class mal::") - 1, sizeof(typeid(T).name())), replace(name, ' ', "%20"), ((results < 25) ? results : 25), page).c_str()
				);
				std::unique_ptr<std::string> all_data = std::make_unique<std::string>();
				all_data->reserve(2048);
				{
					std::unique_ptr<bool> once = std::make_unique<bool>(false);
					std::unique_ptr<char[]> data = std::make_unique<char[]>(2048);
					for (int r = BIO_read(bio.get(), data.get(), sizeof(data) - 1); r > 0; r = BIO_read(bio.get(), data.get(), sizeof(data) - 1)) {
						data[r] = static_cast<char>(0);
						all_data->append(data.get());
						if (not *once and all_data->find("\r\n\r\n") not_eq -1) {
							all_data = std::make_unique<std::string>(all_data->substr(all_data->find("\r\n\r\n") + 4));
							once.reset(new bool(true));
						}
					}
				}
				all_data = std::make_unique<std::string>(all_data->substr(5, all_data->size() - (sizeof("\r\n\r\n") * 2 + sizeof("\n"))));
				std::unique_ptr<json> j = std::make_unique<json>();
				if (json::accept(*all_data))
					*j = json(json::parse(*all_data));
				all_data->clear();
				if (is_null<int>((*j)["pagination"]["items"]["count"]) == 0) break;
				for (const json& data : (*j)["data"])
					callback(T(data));
			}
			});
	}
}
