#include "app/settings_file.h"

#include "Json.h"
#include "JsonFile.h"
#include "PresetFiles.h"

#include <cstdint>

namespace sudoku::app
{

using reckless::core::json::Value;

std::filesystem::path DefaultSettingsFile()
{
    const std::filesystem::path folder = reckless::core::UserSettingsFolder(reckless::core::kSuiteFolder);
    return folder.empty() ? folder : folder / "sudoku" / "settings.json";
}

Settings ReadSettings(const std::filesystem::path &file)
{
    Settings settings;
    if (file.empty())
    {
        return settings;
    }
    const std::optional<Value> document = reckless::core::ReadJsonFile(file);
    if (!document || !document->IsObject())
    {
        return settings;
    }
    settings.shows_pencil_marks = document->BoolOr("pencil_marks", settings.shows_pencil_marks);
    const std::int64_t percent  = document->IntOr("window_percent", settings.window_percent);
    settings.window_percent     = percent >= 50 && percent <= 150 ? static_cast<int>(percent) : settings.window_percent;
    return settings;
}

bool WriteSettings(const std::filesystem::path &file, const Settings &settings)
{
    if (file.empty())
    {
        return false;
    }
    Value document;
    document.Set("pencil_marks", settings.shows_pencil_marks);
    document.Set("window_percent", settings.window_percent);
    return reckless::core::WriteJsonFile(file, document);
}

Settings SettingsKeeper::Read()
{
    seen_ = ReadSettings(file_);
    return *seen_;
}

bool SettingsKeeper::Changed(const Settings &now)
{
    const bool does_differ = !seen_ || !(*seen_ == now);
    if (does_differ && seen_ && WriteSettings(file_, now))
    {
        ++writes_;
    }
    const bool is_first = !has_reported_;
    has_reported_       = true;
    seen_               = now;
    return does_differ || is_first;
}

} // namespace sudoku::app
