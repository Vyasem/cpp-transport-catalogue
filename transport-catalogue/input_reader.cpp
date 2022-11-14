#include "headers/input_reader.h"

#include <iostream>
#include <unordered_map>
#include <tuple>
#include <deque>
#include <string>
#include <algorithm>
#include <vector>

namespace transport {
	namespace input_read {
		namespace detail {
			void TrimString(std::string_view& str) {
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
				auto posEnd = text.npos;
				while (pos != posEnd) {
					int64_t space = text.find(findBy, pos);
					std::string_view tempStr = (space == posEnd ? text.substr(pos) : text.substr(pos, space - pos));
					TrimString(tempStr);
					words.push_back(tempStr);
					pos = text.find_first_not_of(findBy, space);
				}
				return words;
			}
		}

		InputReader::InputReader(catalog::TransportCatalogue* catalog, std::istream& input, int lineCount) :queries(lineCount), catalog_(catalog) {
			for (size_t i = 0; i < queries.size(); ++i) {
				getline(input, queries[i]);
			}
		}

		void InputReader::HandleQuery() {
			for (const std::string& query : queries) {
				auto [left, right] = detail::Split(query, ":");
				auto [queryType, queryName] = detail::Split(left, " ");
				if (queryType == "Bus") {
					detail::TrimString(queryName);
					buses.push_back({ queryName, right });
				}
				else {
					HandleStop(queryName, right);
				}
			}
			HandleBus();
			HandleDistance();
		}

		void InputReader::HandleBus() {
			for (auto& [busName, busStr] : buses) {
				std::string findChar = "-";
				int64_t firstPos = busStr.find("-");
				bool loope = false;
				if (firstPos == busStr.npos) {
					loope = true;
					findChar = ">";
				}
				std::deque<std::string_view> bs = detail::SplitIntoWords(busStr, findChar);
				catalog_->AddRoute(busName, std::move(bs), loope);
			}
		}

		void InputReader::HandleStop(std::string_view stopName, std::string_view stopStr) {
			std::deque<std::string_view> bs = detail::SplitIntoWords(stopStr, ",");
			std::string_view lat = *bs.begin();
			std::string_view lon = *(bs.begin() + 1);
			detail::TrimString(stopName);
			detail::TrimString(lat);
			detail::TrimString(lon);
			catalog_->AddStop(stopName, {stod(std::string(lat)), stod(std::string(lon))});

			if (bs.size() > 2) {
				for (auto it = bs.begin() + 2; it != bs.end(); ++it) {
					auto [distance, stopTo] = detail::Split(*it, " to ");
					detail::TrimString(distance);
					detail::TrimString(stopTo);
					stopsDistance.push_back(std::make_tuple(stopName, stopTo, stoi(std::string(distance))));
				}
			}
		}

		void InputReader::HandleDistance() {
			for (const std::tuple<std::string_view, std::string_view, int>& distanceTuple : stopsDistance) {
				auto [stopFrom, stopTo, distance] = distanceTuple;
				catalog_->SetDistance(stopFrom, stopTo, distance);
			}
		}
	}
}