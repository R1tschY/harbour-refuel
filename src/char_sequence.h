#pragma once

#include <string>
#include <utility>

#include <QString>

namespace _private {
    template<typename Str, typename Seq>
    struct make_char_sequence_impl;

    template<typename Str, size_t... seq>
    struct make_char_sequence_impl<Str, std::integer_sequence<size_t, seq...> > {
        using type = std::integer_sequence<char, Str::get()[seq]...>;
    };
}

template<typename Str>
using make_char_sequence = typename _private::make_char_sequence_impl<
    Str,
    std::make_index_sequence<
        std::char_traits<char>::length(Str::get())>>::type;

template<char... c>
using char_sequence = std::integer_sequence<char, c...>;

template<char... c>
QString charSeqToQString(char_sequence<c...>) {
    QString builder;
    builder.reserve(char_sequence<c...>::size());
    (builder.append(c), ...);
    return builder;
}
