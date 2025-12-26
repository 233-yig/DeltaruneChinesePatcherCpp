#ifndef GAME_VALUES_H
#define GAME_VALUES_H
#include <nlohmann/json.hpp>
namespace Global {
inline const nlohmann::json defaultSettings = {
    {"patcherVersion", "0.9.0"},
    {"displayLanguage", "zh_CN"},
    {"baseWidth", 960},
    {"baseHeight", 720},
    {"qqGroupCode", "1033065757"},
    {"mainFont", "external/main.ttf"},
    {"mainFontSize", 32},
    {"deltaruneAppId", "1671210"},
    {"deltaruneDepotId", "1671212"},
    {"patchRemoteUrl",
     "https://git.ws3917.space:29271/ws3917/deltarune_cnpatch/raw/branch/main"},
    {"patchFileXDelta",
     {
         {"data.win", "main.xdelta"},
         {"chapter1_windows/data.win", "chapter1.xdelta"},
         {"chapter2_windows/data.win", "chapter2.xdelta"},
         {"chapter3_windows/data.win", "chapter3.xdelta"},
         {"chapter4_windows/data.win", "chapter4.xdelta"},
     }},
    {
        "backupFiles",
        {"data.win", "chapter1_windows/data.win", "chapter2_windows/data.win",
         "chapter3_windows/data.win", "chapter4_windows/data.win",
         "chapter3_windows/vid/tennaIntroF1_compressed_28.mp4"},
    },
    {"patchFileStatic",
     {
         "chapter1_windows/lang/lang_en.json",
         "chapter1_windows/lang/lang_en_names.json",
         "chapter1_windows/lang/lang_en_names_recruitable.json",
         "chapter2_windows/lang/lang_en.json",
         "chapter2_windows/lang/lang_en_names.json",
         "chapter2_windows/lang/lang_en_names_recruitable.json",
         "chapter3_windows/lang/lang_en.json",
         "chapter3_windows/lang/lang_en_names.json",
         "chapter3_windows/lang/lang_en_names_recruitable.json",
         "chapter3_windows/vid/tennaIntroF1_compressed_28.mp4",
         "chapter3_windows/vid/tennaIntroF1_zhname_compressed_28.mp4",
         "chapter4_windows/lang/lang_en.json",
         "chapter4_windows/lang/lang_en_names.json",
         "chapter4_windows/lang/lang_en_names_recruitable.json",
     }},
    {"toolSha256",
     {
#ifdef _WIN32
         {"external/win/7z.exe",
          "d2c0045523cf053a6b43f9315e9672fc2535f06aeadd4ffa53c729cd8b2b6dfe"},
         {"external/win/xdelta3.exe",
          "6855c01cf4a1662ba421e6f95370cf9afa2b3ab6c148473c63efe60d634dfb9a"},
#else
         {"external/linux/7z",
          "9a556170350dafb60a97348b86a94b087d97fd36007760691576cac0d88b132b"},
         {"external/linux/xdelta3",
          "709f63ebb9655dc3b5c84f17e11217494eb34cf0009a026386e4c8617ea903"},
#endif
     }}};

} // namespace Global
#endif
