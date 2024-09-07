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
