#include "headers/input_reader.h"

#include<iostream>
#include<unordered_map>
#include<tuple>
#include<deque>
#include<string>
#include <algorithm>
#include<vector>

namespace transport {
	namespace input_read {
		namespace detail {
			void trimString(std::string_view& str) {
				if (str.empty()) {
					return;
				}
				str.remove_prefix(std::min(str.find_first_not_of(" "), str.size()));
				str.remove_suffix(str.size() - str.find_last_not_of(" ") - 1);
			}

			std::pair<std::string_view, std::string_view> Split(std::string_view line, const std::string by) {
				size_t pos = line.find(by);
				std::string_view left = line.substr(0, pos);

				if (pos < line.size() && pos + by.size() < line.size()) {
					return { left, line.substr(pos + by.size()) };
				}
				else {
					return { left, "" };
				}
			}

			std::deque<std::string_view> SplitIntoWords(std::string_view text, const std::string& findBy = " ") {
				std::deque<std::string_view> words;
				int64_t pos = text.find_first_not_of(" ");
				auto pos_end = text.npos;
				while (pos != pos_end) {
					int64_t space = text.find(findBy, pos);
					std::string_view tempStr = (space == pos_end ? text.substr(pos) : text.substr(pos, space - pos));
					trimString(tempStr);
					words.push_back(tempStr);
					pos = text.find_first_not_of(findBy, space);
				}
				return words;
			}
		}

		InputReader::InputReader(catalog::TransportCatalogue* cat, std::istream& input, int lineCount) :queries(lineCount), catalog_(cat) {
			for (size_t i = 0; i < queries.size(); ++i) {
				getline(input, queries[i]);
			}
		}

		void InputReader::queryHandle() {
			for (const std::string& query : queries) {
				auto [left, right] = detail::Split(query, ":");
				auto [queryType, queryName] = detail::Split(left, " ");
				if (queryType == "Bus") {
					detail::trimString(queryName);
					buses.push_back({ queryName, right });
				}
				else {
					stopHandle(queryName, right);
				}
			}
			busHandle();
			distanceHandle();
		}

		void InputReader::busHandle() {
			for (auto& [busName, busStr] : buses) {
				std::string findChar = "-";
				int64_t firstPos = busStr.find("-");
				bool loope = false;
				if (firstPos == busStr.npos) {
					loope = true;
					findChar = ">";
				}
				std::deque<std::string_view> bs = detail::SplitIntoWords(busStr, findChar);
				catalog_->AddRoute(busName, bs, loope);
			}
		}

		void InputReader::stopHandle(std::string_view stopName, std::string_view stopStr) {
			std::deque<std::string_view> bs = detail::SplitIntoWords(stopStr, ",");
			std::string_view lat = *bs.begin();
			std::string_view lon = *(bs.begin() + 1);
			detail::trimString(stopName);
			detail::trimString(lat);
			detail::trimString(lon);
			catalog_->AddStop(stopName, stod(std::string(lat)), stod(std::string(lon)));

			if (bs.size() > 2) {
				for (auto it = bs.begin() + 2; it != bs.end(); ++it) {
					auto [distance, stopTo] = detail::Split(*it, " to ");
					detail::trimString(distance);
					detail::trimString(stopTo);
					stopsDistance.push_back(std::make_tuple(stopName, stopTo, stoi(std::string(distance))));
				}
			}
		}

		void InputReader::distanceHandle() {
			for (std::tuple<std::string_view, std::string_view, int> distance : stopsDistance) {
				catalog_->SetDistance(std::move(distance));
			}
		}
	}
}