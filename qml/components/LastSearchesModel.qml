import QtQuick 2.0
import QtQuick.LocalStorage 2.0
import Sailfish.Silica 1.0

import de.richardliebscher.refuel 0.1


QtObject {
    property DbConnection db

    signal itemsChanged()

    function add(timestamp, provider, name, coordinate, fuel, distance) {
        db.transaction(function(tx) {
            tx.executeSql(
                "INSERT INTO last_searches (
                    timestamp, provider, name, latitude, longitude, fuel, distance)
                 VALUES (
                    ?, ?, ?, ?, ?, ?, ?)
                 ON CONFLICT(
                    provider, name, latitude, longitude, fuel)
                 DO UPDATE SET
                    timestamp=excluded.timestamp, distance=excluded.distance",
                [timestamp, provider, name,
                 coordinate.latitude, coordinate.longitude, fuel, distance])
        })

        itemsChanged()
    }

    function removeByRowId(rowId) {
        db.transaction(function(tx) {
            tx.executeSql(
                "DELETE FROM last_searches WHERE _rowid_ = ?",
                [rowId])
        })

        itemsChanged()
    }
}
