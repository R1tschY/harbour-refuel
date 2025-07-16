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

QtObject {
    property string dataBaseId

    property var _db

    onDataBaseIdChanged: {      
        if (dataBaseId) {
            _db = LocalStorage.openDatabaseSync(dataBaseId, "", "", 1000000)
            if (_db.version === "") {
                _db.changeVersion("", "1", function(tx) {
                    tx.executeSql(
                                "CREATE TABLE last_searches (" +
                                "  timestamp INTEGER NOT NULL," +
                                "  provider TEXT," +
                                "  name TEXT NOT NULL," +
                                "  latitude REAL," +
                                "  longitude REAL," +
                                "  fuel INT," +
                                "  distance REAL," +
                                "  UNIQUE(provider, name, latitude, longitude, fuel)" +
                                ")");
                })
            } else if (_db.version === "1") {
                _db.changeVersion("1", "2", function(tx) {
                    tx.executeSql("DROP TABLE last_searches");
                    tx.executeSql(
                                "CREATE TABLE last_searches (" +
                                "  timestamp INTEGER NOT NULL," +
                                "  provider TEXT," +
                                "  name TEXT NOT NULL," +
                                "  latitude REAL," +
                                "  longitude REAL," +
                                "  fuel_id TEXT," +
                                "  distance REAL," +
                                "  UNIQUE(provider, name, latitude, longitude, fuel_id)" +
                                ")");
                })
            }
        } else {
            _db = null
        }
    }

    function transaction(fn) {
        if (_db !== null) {
            _db.transaction(fn)
        }
    }
}
