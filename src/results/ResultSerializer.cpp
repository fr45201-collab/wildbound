#include "ResultSerializer.hpp"
#include "core/Logger.hpp"

#include <nlohmann/json.hpp>
#include <filesystem>
#include <fstream>

namespace wildbound::results {

namespace fs = std::filesystem;
using json = nlohmann::json;

bool ResultSerializer::SaveResult(const MatchResultModel& model,
                                 const std::string& directoryPath) const {
    if (!model.isFinalized) {
        core::Logger::Warn("RESULTS", "Refusing to persist unfinalized match result.");
        return false;
    }

    try {
        if (!fs::exists(directoryPath)) {
            fs::create_directories(directoryPath);
        }

        std::string filename = model.matchId;
        if (filename.empty()) {
            filename = "match_" + std::to_string(static_cast<uint64_t>(model.matchDuration));
        }

        // Sanitize filename
        for (char& c : filename) {
            if (c == '/' || c == '\\' || c == ':' || c == '*' || c == '?' || c == '"' || c == '<' || c == '>' || c == '|') {
                c = '_';
            }
        }

        fs::path fullPath = fs::path(directoryPath) / (filename + ".json");

        json j;
        j["match_id"] = model.matchId;
        j["match_outcome"] = MatchOutcomeToString(model.matchOutcome);
        j["local_player_outcome"] = PlayerOutcomeToString(model.localPlayerOutcome);
        j["winner_participant_id"] = model.winnerParticipantId;
        j["winner_display_name"] = model.winnerDisplayName;
        j["match_duration"] = model.matchDuration;
        j["total_participants"] = model.totalParticipants;
        j["total_eliminations"] = model.totalEliminations;
        j["survivors_count"] = model.survivorsCount;
        j["completion_reason"] = model.completionReason;

        // Local Player Performance
        const auto& lp = model.localPlayerPerformance;
        j["local_player"] = {
            { "participant_id", lp.participantId },
            { "display_name", lp.displayName },
            { "type", static_cast<int>(lp.type) },
            { "outcome", PlayerOutcomeToString(lp.outcome) },
            { "placement", lp.placement },
            { "survival_duration", lp.survivalDuration },
            { "kills", lp.kills },
            { "deaths", lp.deaths },
            { "damage_dealt", lp.damageDealt },
            { "combat_damage_received", lp.combatDamageReceived },
            { "storm_damage_received", lp.stormDamageReceived },
            { "total_damage_received", lp.totalDamageReceived },
            { "elimination_cause", static_cast<int>(lp.eliminationCause) },
            { "killer_participant_id", lp.killerParticipantId },
            { "killer_display_name", lp.killerDisplayName },
            { "is_winner", lp.isWinner }
        };

        // Leaderboard
        j["leaderboard"] = json::array();
        for (const auto& p : model.participantPerformances) {
            j["leaderboard"].push_back({
                { "participant_id", p.participantId },
                { "display_name", p.displayName },
                { "type", static_cast<int>(p.type) },
                { "outcome", PlayerOutcomeToString(p.outcome) },
                { "placement", p.placement },
                { "survival_duration", p.survivalDuration },
                { "kills", p.kills },
                { "deaths", p.deaths },
                { "damage_dealt", p.damageDealt },
                { "combat_damage_received", p.combatDamageReceived },
                { "storm_damage_received", p.stormDamageReceived },
                { "total_damage_received", p.totalDamageReceived },
                { "elimination_cause", static_cast<int>(p.eliminationCause) },
                { "killer_participant_id", p.killerParticipantId },
                { "killer_display_name", p.killerDisplayName },
                { "is_winner", p.isWinner }
            });
        }

        // Elimination Records
        j["eliminations"] = json::array();
        for (const auto& elim : model.eliminationRecords) {
            j["eliminations"].push_back({
                { "event_id", elim.eventId },
                { "victim_id", elim.victimParticipantId },
                { "victim_name", elim.victimName },
                { "killer_id", elim.killerParticipantId },
                { "killer_name", elim.killerName },
                { "cause", static_cast<int>(elim.cause) },
                { "match_time", elim.matchElapsedTime },
                { "placement", elim.placement }
            });
        }

        std::ofstream file(fullPath);
        if (!file.is_open()) {
            core::Logger::Error("RESULTS", "Failed to open match history file for write: " + fullPath.string());
            return false;
        }

        file << j.dump(4);
        core::Logger::Info("RESULTS", "Successfully saved match history to: " + fullPath.string());
        return true;
    } catch (const std::exception& e) {
        core::Logger::Error("RESULTS", "Exception while saving match history: " + std::string(e.what()));
        return false;
    }
}

bool ResultSerializer::LoadResult(const std::string& filePath, MatchResultModel& outModel) const {
    outModel.Reset();

    if (!fs::exists(filePath)) {
        core::Logger::Warn("RESULTS", "Match history file does not exist: " + filePath);
        return false;
    }

    try {
        std::ifstream file(filePath);
        if (!file.is_open()) {
            core::Logger::Error("RESULTS", "Failed to open match history file for read: " + filePath);
            return false;
        }

        json j;
        file >> j;

        outModel.matchId = j.value("match_id", "");
        std::string outcomeStr = j.value("match_outcome", "InProgress");
        if (outcomeStr == "Victory") outModel.matchOutcome = MatchOutcome::Victory;
        else if (outcomeStr == "Draw") outModel.matchOutcome = MatchOutcome::Draw;
        else if (outcomeStr == "Cancelled") outModel.matchOutcome = MatchOutcome::Cancelled;
        else if (outcomeStr == "Failed") outModel.matchOutcome = MatchOutcome::Failed;
        else outModel.matchOutcome = MatchOutcome::InProgress;

        std::string localOutcomeStr = j.value("local_player_outcome", "Undecided");
        if (localOutcomeStr == "Victory") outModel.localPlayerOutcome = PlayerOutcome::Victory;
        else if (localOutcomeStr == "Defeat") outModel.localPlayerOutcome = PlayerOutcome::Defeat;
        else if (localOutcomeStr == "Draw") outModel.localPlayerOutcome = PlayerOutcome::Draw;
        else outModel.localPlayerOutcome = PlayerOutcome::Undecided;

        outModel.winnerParticipantId = j.value("winner_participant_id", 0u);
        outModel.winnerDisplayName = j.value("winner_display_name", "No Winner (Draw)");
        outModel.matchDuration = j.value("match_duration", 0.0f);
        outModel.totalParticipants = j.value("total_participants", 0u);
        outModel.totalEliminations = j.value("total_eliminations", 0u);
        outModel.survivorsCount = j.value("survivors_count", 0u);
        outModel.completionReason = j.value("completion_reason", "");

        // Local Player
        if (j.contains("local_player")) {
            const auto& lp = j["local_player"];
            outModel.localPlayerPerformance.participantId = lp.value("participant_id", 0u);
            outModel.localPlayerPerformance.displayName = lp.value("display_name", "");
            outModel.localPlayerPerformance.type = static_cast<match::ParticipantType>(lp.value("type", 0));
            outModel.localPlayerPerformance.outcome = outModel.localPlayerOutcome;
            outModel.localPlayerPerformance.placement = lp.value("placement", 0u);
            outModel.localPlayerPerformance.survivalDuration = lp.value("survival_duration", 0.0f);
            outModel.localPlayerPerformance.kills = lp.value("kills", 0u);
            outModel.localPlayerPerformance.deaths = lp.value("deaths", 0u);
            outModel.localPlayerPerformance.damageDealt = lp.value("damage_dealt", 0.0f);
            outModel.localPlayerPerformance.combatDamageReceived = lp.value("combat_damage_received", 0.0f);
            outModel.localPlayerPerformance.stormDamageReceived = lp.value("storm_damage_received", 0.0f);
            outModel.localPlayerPerformance.totalDamageReceived = lp.value("total_damage_received", 0.0f);
            outModel.localPlayerPerformance.eliminationCause = static_cast<match::EliminationCause>(lp.value("elimination_cause", 0));
            outModel.localPlayerPerformance.killerParticipantId = lp.value("killer_participant_id", 0u);
            outModel.localPlayerPerformance.killerDisplayName = lp.value("killer_display_name", "");
            outModel.localPlayerPerformance.isWinner = lp.value("is_winner", false);
        }

        // Leaderboard
        if (j.contains("leaderboard") && j["leaderboard"].is_array()) {
            for (const auto& item : j["leaderboard"]) {
                PlayerPerformance p;
                p.participantId = item.value("participant_id", 0u);
                p.displayName = item.value("display_name", "");
                p.type = static_cast<match::ParticipantType>(item.value("type", 0));
                std::string pOutStr = item.value("outcome", "Undecided");
                if (pOutStr == "Victory") p.outcome = PlayerOutcome::Victory;
                else if (pOutStr == "Defeat") p.outcome = PlayerOutcome::Defeat;
                else if (pOutStr == "Draw") p.outcome = PlayerOutcome::Draw;
                else p.outcome = PlayerOutcome::Undecided;
                p.placement = item.value("placement", 0u);
                p.survivalDuration = item.value("survival_duration", 0.0f);
                p.kills = item.value("kills", 0u);
                p.deaths = item.value("deaths", 0u);
                p.damageDealt = item.value("damage_dealt", 0.0f);
                p.combatDamageReceived = item.value("combat_damage_received", 0.0f);
                p.stormDamageReceived = item.value("storm_damage_received", 0.0f);
                p.totalDamageReceived = item.value("total_damage_received", 0.0f);
                p.eliminationCause = static_cast<match::EliminationCause>(item.value("elimination_cause", 0));
                p.killerParticipantId = item.value("killer_participant_id", 0u);
                p.killerDisplayName = item.value("killer_display_name", "");
                p.isWinner = item.value("is_winner", false);
                outModel.participantPerformances.push_back(p);
            }
        }

        // Eliminations
        if (j.contains("eliminations") && j["eliminations"].is_array()) {
            for (const auto& item : j["eliminations"]) {
                EliminationRecord rec;
                rec.eventId = item.value("event_id", 0u);
                rec.victimParticipantId = item.value("victim_id", 0u);
                rec.victimName = item.value("victim_name", "");
                rec.killerParticipantId = item.value("killer_id", 0u);
                rec.killerName = item.value("killer_name", "");
                rec.cause = static_cast<match::EliminationCause>(item.value("cause", 0));
                rec.matchElapsedTime = item.value("match_time", 0.0f);
                rec.placement = item.value("placement", 0u);
                outModel.eliminationRecords.push_back(rec);
            }
        }

        outModel.isFinalized = true;
        return true;
    } catch (const std::exception& e) {
        core::Logger::Error("RESULTS", "Exception while loading match history: " + std::string(e.what()));
        return false;
    }
}

std::vector<std::string> ResultSerializer::GetMatchHistoryFiles(const std::string& directoryPath) const {
    std::vector<std::string> files;
    if (!fs::exists(directoryPath)) {
        return files;
    }

    try {
        for (const auto& entry : fs::directory_iterator(directoryPath)) {
            if (entry.is_regular_file() && entry.path().extension() == ".json") {
                files.push_back(entry.path().string());
            }
        }
        std::sort(files.begin(), files.end());
    } catch (const std::exception& e) {
        core::Logger::Error("RESULTS", "Failed to iterate match history directory: " + std::string(e.what()));
    }

    return files;
}

} // namespace wildbound::results

