/*
 * Copyright 2025 Richard Liebscher <r1tschy@posteo.de>.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
 
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
