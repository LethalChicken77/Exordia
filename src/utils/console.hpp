#pragma once
#include <string>
#include <queue>
#include <iostream>

class Console
{
    struct ConsoleMessage
    {
        std::string message;
        enum Type
        {
            NONE = 0,
            INFO = 1,
            DEBUG_T = 2, // DEBUG is already defined
            WARNING = 3,
            ERROR = 4
        } type;
    };

    public:
        static void logRaw(std::string_view message, bool terminalOnly = false);
        static void log(const std::string_view message, const std::string_view source = "", bool terminalOnly = false);
        static void debug(const std::string_view message, const std::string_view source = "", bool terminalOnly = false);
        static void warn(const std::string_view message, const std::string_view source = "", bool terminalOnly = false);
        static void error(const std::string_view message, const std::string_view source = "", bool terminalOnly = false);

        static void clear();
        static void drawImGui();

        static void saveLog();
        static void saveLogError();
    private:
        static const size_t maxMessages = 100; // If exceeded, remove oldest
        static std::queue<ConsoleMessage> messages;
        static bool scrollToBottom;

        static ConsoleMessage constructMessage(const std::string_view message, const std::string_view source, ConsoleMessage::Type type);
        static void pushMessage(ConsoleMessage& message);

        static inline constexpr std::string consoleEndl = "\033[0m\n";
        static inline constexpr std::string ANSIreset = "\033[0m";
        static inline constexpr std::string ANSIred = "\033[31m";
        static inline constexpr std::string ANSIyellow = "\033[33m";
        static inline constexpr std::string ANSIgray = "\033[90m";
        static inline constexpr std::string ANSIgreen = "\033[32m";
    public:
        // Putting logf functions down here for readability

        /// @brief Print to the console using std::format
        /// @param fmt Format string, same rules as std::format
        /// @param args Values to be inserted into the format string
        template<typename... _Args>
        static inline void logf(const std::format_string<_Args...> &fmt, _Args&&... args)
        {
            log(std::format(fmt, std::forward<_Args>(args)...));
        }
        template<typename... _Args>
        static inline void logf(const std::format_string<_Args...> &fmt, _Args&&... args, const std::string_view source)
        {
            log(std::format(fmt, std::forward<_Args>(args)...), source);
        }
        template<typename... _Args>
        static inline void logf(const std::format_string<_Args...> &fmt, _Args&&... args, const std::string_view source, bool terminalOnly)
        {
            log(std::format(fmt, std::forward<_Args>(args)...), source, terminalOnly);
        }

        /// @brief Print debug to the console using std::format
        /// @param fmt Format string, same rules as std::format
        /// @param args Values to be inserted into the format string
        template<typename... _Args>
        static inline void debugf(const std::format_string<_Args...> &fmt, _Args&&... args)
        {
            debug(std::format(fmt, std::forward<_Args>(args)...));
        }
        template<typename... _Args>
        static inline void debugf(const std::format_string<_Args...> &fmt, _Args&&... args, const std::string_view source)
        {
            debug(std::format(fmt, std::forward<_Args>(args)...), source);
        }
        template<typename... _Args>
        static inline void debugf(const std::format_string<_Args...> &fmt, _Args&&... args, const std::string_view source, bool terminalOnly)
        {
            debug(std::format(fmt, std::forward<_Args>(args)...), source, terminalOnly);
        }
        
        /// @brief Print warning to the console using std::format
        /// @param fmt Format string, same rules as std::format
        /// @param args Values to be inserted into the format string
        template<typename... _Args>
        static inline void warnf(const std::format_string<_Args...> &fmt, _Args&&... args)
        {
            warn(std::format(fmt, std::forward<_Args>(args)...));
        }
        template<typename... _Args>
        static inline void warnf(const std::format_string<_Args...> &fmt, _Args&&... args, const std::string_view source)
        {
            warn(std::format(fmt, std::forward<_Args>(args)...), source);
        }
        template<typename... _Args>
        static inline void warnf(const std::format_string<_Args...> &fmt, _Args&&... args, const std::string_view source, bool terminalOnly)
        {
            warn(std::format(fmt, std::forward<_Args>(args)...), source, terminalOnly);
        }
        
        /// @brief Print error to the console using std::format
        /// @param fmt Format string, same rules as std::format
        /// @param args Values to be inserted into the format string
        template<typename... _Args>
        static inline void errorf(const std::format_string<_Args...> &fmt, _Args&&... args)
        {
            error(std::format(fmt, std::forward<_Args>(args)...));
        }
        template<typename... _Args>
        static inline void errorf(const std::format_string<_Args...> &fmt, _Args&&... args, const std::string_view source)
        {
            error(std::format(fmt, std::forward<_Args>(args)...), source);
        }
        template<typename... _Args>
        static inline void errorf(const std::format_string<_Args...> &fmt, _Args&&... args, const std::string_view source, bool terminalOnly)
        {
            error(std::format(fmt, std::forward<_Args>(args)...), source, terminalOnly);
        }
        
};