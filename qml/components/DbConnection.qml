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
    property int _version: 0

    onDataBaseIdChanged: {      
        if (dataBaseId) {
            this._db = LocalStorage.openDatabaseSync(this.dataBaseId, "", "", 1000000)
            console.log("Got database version '" + this._db.version + "'")
            this._version = this._db.version ? parseInt(this._db.version) : 0

            this._upgrade(0, 1, function(tx) {
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

            this._upgrade(1, 2, function(tx) {
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

            this._upgrade(2, 3, function(tx) {
                tx.executeSql(
                            "CREATE TABLE favourites (" +
                            "  provider TEXT," +
                            "  id TEXT NOT NULL," +
                            "  brand TEXT," +
                            "  name TEXT," +
                            "  address TEXT," +
                            "  fuel_id TEXT NOT NULL," +
                            "  \"order\" INTEGER NOT NULL," +
                            "  UNIQUE(provider, id, fuel_id)" +
                            ")");
                tx.executeSql(
                            "CREATE TABLE alerts (" +
                            "  favourite INTEGER NOT NULL," +
                            "  deadline INTEGER," +
                            "  \"limit\" REAL," +
                            "  UNIQUE(favourite, deadline, \"limit\")" +
                            ")");
            })
        } else {
            this._db = null
        }
    }

    function transaction(fn) {
        if (this._db !== null) {
            this._db.transaction(fn)
        }
    }

    function _upgrade(from, to, fn) {
        if (this._db !== null) {
            if (this._version === from) {
                console.log("Upgrading database to version '" + to + "'")
                this._db.changeVersion(
                            from === 0 ? "" : from.toString(),
                            to.toString(),
                            fn)
                this._db = LocalStorage.openDatabaseSync(this.dataBaseId, "", "", 1000000)
                console.log("Upgraded database to version '" + to + "'")
                this._version = to;
            }
        }
    }
}
