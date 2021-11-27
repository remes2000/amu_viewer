module pl.nieruchalski.client {
    requires javafx.controls;
    requires javafx.fxml;
    requires commons.validator;

    opens pl.nieruchalski.client to javafx.fxml;
    exports pl.nieruchalski.client;
}