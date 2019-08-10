#ifndef MIDI2LR_MISC_H_INCLUDED
#define MIDI2LR_MISC_H_INCLUDED
/*
==============================================================================

Misc.h

This file is part of MIDI2LR. Copyright 2015 by Rory Jaffe.

MIDI2LR is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later
version.

MIDI2LR is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
MIDI2LR.  If not, see <http://www.gnu.org/licenses/>.
==============================================================================
*/
#include <chrono>
#include <exception>
#include <string>
// ReSharper disable once CppUnusedIncludeDirective
#include <string_view>
#include <thread>   //sleep_for
#include <typeinfo> //for typeid, used in calls to ExceptionResponse

#include <JuceLibraryCode/JuceHeader.h>
#include <gsl/gsl>

#ifdef NDEBUG // asserts disabled
static constexpr bool kNdebug = true;
#else // asserts enabled
static constexpr bool kNdebug = false;
#endif

#ifndef NDEBUG
#ifdef _WIN32
// declarations from processthreadsapi.h
extern "C" __declspec(dllimport) long __stdcall SetThreadDescription(
    _In_ void* hThread, _In_ const wchar_t* lpThreadDescription);
extern "C" __declspec(dllimport) void* __stdcall GetCurrentThread();
namespace rsj {
   inline void LabelThread(const wchar_t* threadname) noexcept
   {
      SetThreadDescription(GetCurrentThread(), threadname);
   }
} // namespace rsj
#else
namespace rsj {
   constexpr void LabelThread([[maybe_unused]] const wchar_t* threadname) noexcept
   {
      /*nothing*/;
   }
} // namespace rsj
#endif
#else
namespace rsj {
   constexpr void LabelThread([[maybe_unused]] const wchar_t* threadname) noexcept
   {
      /*nothing*/;
   }
} // namespace rsj
#endif

#ifdef _WIN32
constexpr auto MSWindows{true};
constexpr auto OSX{false};
#else
constexpr auto MSWindows{false};
constexpr auto OSX{true};
#endif

namespace rsj {
   [[nodiscard]] std::string ReplaceInvisibleChars(std::string_view in);
   [[nodiscard]] bool EndsWith(std::string_view main_str, std::string_view to_match);
   // typical call: rsj::ExceptionResponse(typeid(this).name(), __func__, e);
   void ExceptionResponse(gsl::czstring<> id, gsl::czstring<> fu, const std::exception& e) noexcept;
   // char* overloads here are to allow catch clauses to avoid a juce::String conversion at the
   // caller location, thus avoiding a potential exception in the catch clause. string_view
   // overloads not used because those are ambiguous with the String versions.
   void LogAndAlertError(const juce::String& error_text) noexcept;
   void LogAndAlertError(gsl::czstring<> error_text) noexcept;
   void Log(const juce::String& info) noexcept;
   void Log(gsl::czstring<> info) noexcept;
   [[nodiscard]] std::string ToLower(std::string_view in);
#ifdef _WIN32
   [[nodiscard]] std::wstring AppDataFilePath(std::wstring_view file_name);
   [[nodiscard]] std::wstring AppDataFilePath(std::string_view file_name);
   [[nodiscard]] inline std::wstring AppLogFilePath(const std::wstring& file_name)
   {
      return AppDataFilePath(file_name);
   }
   [[nodiscard]] inline std::wstring AppLogFilePath(const std::string& file_name)
   {
      return AppDataFilePath(file_name);
   }
   [[nodiscard]] std::wstring Utf8ToWide(std::string_view in);
   [[nodiscard]] std::string WideToUtf8(std::wstring_view in);
#else
   [[nodiscard]] std::string AppDataFilePath(const std::string& file_name);
   [[nodiscard]] std::string AppLogFilePath(const std::string& file_name);
#endif // def _WIN32

   // -------------------------------------------------------------------
   // --- Reversed iterable

   // https://stackoverflow.com/a/42221253/5699329
   template<class T> struct ReverseWrapper {
      T o;
      ReverseWrapper(T&& i) : o(std::forward<T>(i)) {}
   };

   template<class T> auto begin(ReverseWrapper<T>& r)
   {
      using std::end;
      return std::make_reverse_iterator(end(r.o));
   }

   template<class T> auto end(ReverseWrapper<T>& r)
   {
      using std::begin;
      return std::make_reverse_iterator(begin(r.o));
   }

   template<class T> auto begin(ReverseWrapper<T> const& r)
   {
      using std::end;
      return std::make_reverse_iterator(end(r.o));
   }

   template<class T> auto end(ReverseWrapper<T> const& r)
   {
      using std::begin;
      return std::make_reverse_iterator(begin(r.o));
   }

   template<class T> auto Reverse(T&& ob)
   {
      return ReverseWrapper<T>{std::forward<T>(ob)};
   }

#pragma warning(push)
#pragma warning(disable : 4127) // constant conditional expression
   // zepto yocto zetta and yotta too large/small to be represented by intmax_t
   // TODO: change to consteval, find way to convert digit to string for unexpected
   // values, so return could be, e.g., "23425/125557 ", instead of error message
   template<class R>[[nodiscard]] constexpr auto RatioToPrefix() noexcept
   {
      if (R::num == 1) {
         switch (R::den) {
         case 1:
            return "";
         case 10:
            return "deci";
         case 100:
            return "centi";
         case 1000:
            return "milli";
         case 1000000:
            return "micro";
         case 1000000000:
            return "nano";
         case 1000000000000:
            return "pico";
         case 1000000000000000:
            return "femto";
         case 1000000000000000000:
            return "atto";
         default:
             /* empty */;
         }
      }
      if (R::den == 1) {
         switch (R::num) {
         case 10:
            return "deca";
         case 100:
            return "hecto";
         case 1000:
            return "kilo";
         case 1000000:
            return "mega";
         case 1000000000:
            return "giga";
         case 1000000000000:
            return "tera";
         case 1000000000000000:
            return "peta";
         case 1000000000000000000:
            return "exa";
         default:
             /* empty */;
         }
      }
      return "unexpected ratio encountered ";
   }
#pragma warning(pop)

   template<class Rep, class Period>
   auto SleepTimed(const std::chrono::duration<Rep, Period> sleep_duration) noexcept //-V801
   {
      const auto start = std::chrono::high_resolution_clock::now();
      std::this_thread::sleep_for(sleep_duration);
      const auto end = std::chrono::high_resolution_clock::now();
      const std::chrono::duration<double, Period> elapsed = end - start;
      return elapsed;
   }

   template<class Rep, class Period>
   void SleepTimedLogged(
       std::string_view msg_prefix, const std::chrono::duration<Rep, Period> sleep_duration) //-V801
   {
      const auto elapsed = SleepTimed(sleep_duration);
      rsj::Log(juce::String(msg_prefix.data(), msg_prefix.size()) + " thread slept for "
               + juce::String(elapsed.count()) + ' ' + RatioToPrefix<Period>() + "seconds.");
   }
} // namespace rsj

#endif // MISC_H_INCLUDED
