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

import QtQuick 2.0
import QtQuick.LocalStorage 2.0
import Sailfish.Silica 1.0

import de.richardliebscher.refuel 0.1


QtObject {
    property DbConnection db

    signal itemsChanged()

    function add(provider, id, brand, name, address, fuel) {
        db.transaction(function(tx) {
            tx.executeSql(
                "INSERT OR REPLACE INTO favourites (
                    provider, id, brand, name, address, fuel_id, \"order\")
                 VALUES (?, ?, ?, ?, ?, ?, (SELECT count(*) FROM favourites))",
                [provider, id, brand, name, address, fuel])
        })

        itemsChanged()
    }

    function getForStation(provider, id) {
        var res = []
        db.transaction(function(tx) {
            var results = tx.executeSql(
                "SELECT fuel_id \
                 FROM favourites\
                 WHERE provider = ? AND id = ?",
                [provider, id])
            var length = results.rows.length
            for (var i = 0; i < length; i++) {
                res.push(results.rows.item(i).fuel_id)
            }
        })
        return res
    }

    function remove(provider, id, fuelId) {
        db.transaction(function(tx) {
            tx.executeSql(
                "DELETE FROM favourites WHERE provider = ? AND id = ? AND fuel_id = ?",
                [provider, id, fuelId])
        })

        itemsChanged()
    }
}
