#pragma once

#include "MatchResultModel.hpp"
#include <string>
#include <vector>

namespace wildbound::results {

/**
 * Local JSON persistence system for post-match history.
 * Serializes and deserializes immutable match records in user_data/match_history/.
 */
class ResultSerializer {
public:
    ResultSerializer() = default;
    ~ResultSerializer() = default;

    /**
     * Serializes a finalized MatchResultModel to a JSON file.
     * @param model The immutable match result model.
     * @param directoryPath Directory to store history (defaults to user_data/match_history).
     * @return True if file was written successfully, false otherwise.
     */
    bool SaveResult(const MatchResultModel& model,
                    const std::string& directoryPath = "user_data/match_history") const;

    /**
     * Loads a MatchResultModel from a saved JSON file.
     * @param filePath Full path to the match history JSON file.
     * @param outModel Outputs parsed result model.
     * @return True if parsed and validated successfully.
     */
    bool LoadResult(const std::string& filePath, MatchResultModel& outModel) const;

    /**
     * Retrieves all saved match history JSON file paths.
     */
    [[nodiscard]] std::vector<std::string> GetMatchHistoryFiles(
        const std::string& directoryPath = "user_data/match_history"
    ) const;
};

} // namespace wildbound::results

