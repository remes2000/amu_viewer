package pl.nieruchalski.client.domain.helpers;

import javafx.scene.input.KeyCode;

import java.security.Key;
import java.util.HashMap;
import java.util.Map;

public class KeyCodeMap {
    private static HashMap<KeyCode, Integer> keyCodeMap;
    static {
        keyCodeMap = new HashMap();
        keyCodeMap.put(KeyCode.ESCAPE, 9);
        keyCodeMap.put(KeyCode.F1, 67);
        keyCodeMap.put(KeyCode.F2, 68);
        keyCodeMap.put(KeyCode.F3, 69);
        keyCodeMap.put(KeyCode.F4, 70);
        keyCodeMap.put(KeyCode.F5, 71);
        keyCodeMap.put(KeyCode.F6, 72);
        keyCodeMap.put(KeyCode.F7, 73);
        keyCodeMap.put(KeyCode.F8, 74);
        keyCodeMap.put(KeyCode.F9, 75);
        keyCodeMap.put(KeyCode.F10, 76);
        keyCodeMap.put(KeyCode.F11, 95);
        keyCodeMap.put(KeyCode.F12, 96);
        keyCodeMap.put(KeyCode.BACK_QUOTE, 49);
        keyCodeMap.put(KeyCode.DIGIT1, 10);
        keyCodeMap.put(KeyCode.DIGIT2, 11);
        keyCodeMap.put(KeyCode.DIGIT3, 12);
        keyCodeMap.put(KeyCode.DIGIT4, 13);
        keyCodeMap.put(KeyCode.DIGIT5, 14);
        keyCodeMap.put(KeyCode.DIGIT6, 15);
        keyCodeMap.put(KeyCode.DIGIT7, 16);
        keyCodeMap.put(KeyCode.DIGIT8, 17);
        keyCodeMap.put(KeyCode.DIGIT9, 18);
        keyCodeMap.put(KeyCode.DIGIT0, 19);
        keyCodeMap.put(KeyCode.MINUS, 20);
        keyCodeMap.put(KeyCode.EQUALS, 21);
        keyCodeMap.put(KeyCode.BACK_SPACE, 22);
        keyCodeMap.put(KeyCode.TAB, 23);
        keyCodeMap.put(KeyCode.Q, 24);
        keyCodeMap.put(KeyCode.W, 25);
        keyCodeMap.put(KeyCode.E, 26);
        keyCodeMap.put(KeyCode.R, 27);
        keyCodeMap.put(KeyCode.T, 28);
        keyCodeMap.put(KeyCode.Y, 29);
        keyCodeMap.put(KeyCode.U, 30);
        keyCodeMap.put(KeyCode.I, 31);
        keyCodeMap.put(KeyCode.O, 32);
        keyCodeMap.put(KeyCode.P, 33);
        keyCodeMap.put(KeyCode.OPEN_BRACKET, 34);
        keyCodeMap.put(KeyCode.CLOSE_BRACKET, 35);
        keyCodeMap.put(KeyCode.ENTER, 36);
        keyCodeMap.put(KeyCode.CAPS, 66);
        keyCodeMap.put(KeyCode.A, 38);
        keyCodeMap.put(KeyCode.S, 39);
        keyCodeMap.put(KeyCode.D, 40);
        keyCodeMap.put(KeyCode.F, 41);
        keyCodeMap.put(KeyCode.G, 42);
        keyCodeMap.put(KeyCode.H, 43);
        keyCodeMap.put(KeyCode.J, 44);
        keyCodeMap.put(KeyCode.K, 45);
        keyCodeMap.put(KeyCode.L, 46);
        keyCodeMap.put(KeyCode.SEMICOLON, 47);
        keyCodeMap.put(KeyCode.QUOTE, 48);
        keyCodeMap.put(KeyCode.BACK_SLASH, 51);
        keyCodeMap.put(KeyCode.LESS, 94);
        keyCodeMap.put(KeyCode.Z, 52);
        keyCodeMap.put(KeyCode.X, 53);
        keyCodeMap.put(KeyCode.C, 54);
        keyCodeMap.put(KeyCode.V, 55);
        keyCodeMap.put(KeyCode.B, 56);
        keyCodeMap.put(KeyCode.N, 57);
        keyCodeMap.put(KeyCode.M, 58);
        keyCodeMap.put(KeyCode.COMMA, 59);
        keyCodeMap.put(KeyCode.PERIOD, 60);
        keyCodeMap.put(KeyCode.SLASH, 61);
        keyCodeMap.put(KeyCode.SHIFT, 50); // LEFT = 50 RIGHT = 62
        keyCodeMap.put(KeyCode.CONTROL, 37); // LEFT = 37 RIGHT = 105
        keyCodeMap.put(KeyCode.WINDOWS, 133); // LEFT = 133 RIGHT = 134
        keyCodeMap.put(KeyCode.ALT, 64);
        keyCodeMap.put(KeyCode.SPACE, 65);
        // end of basic keyboard keys
        keyCodeMap.put(KeyCode.PRINTSCREEN, 107);
        keyCodeMap.put(KeyCode.INSERT, 118);
        keyCodeMap.put(KeyCode.HOME, 110);
        keyCodeMap.put(KeyCode.PAGE_UP, 185);
        keyCodeMap.put(KeyCode.PAGE_DOWN, 186);
        keyCodeMap.put(KeyCode.DELETE, 119);
        keyCodeMap.put(KeyCode.END, 115);
        // ARROWS
        keyCodeMap.put(KeyCode.LEFT, 113);
        keyCodeMap.put(KeyCode.RIGHT, 114);
        keyCodeMap.put(KeyCode.UP, 111);
        keyCodeMap.put(KeyCode.DOWN, 116);
        // NUMPAD
        keyCodeMap.put(KeyCode.NUM_LOCK, 77);
        keyCodeMap.put(KeyCode.DIVIDE, 106);
        keyCodeMap.put(KeyCode.MULTIPLY, 63);
        keyCodeMap.put(KeyCode.SUBTRACT, 82);
        keyCodeMap.put(KeyCode.ADD, 86);
        keyCodeMap.put(KeyCode.SEPARATOR, 91);
        keyCodeMap.put(KeyCode.NUMPAD7, 79);
        keyCodeMap.put(KeyCode.NUMPAD8, 80);
        keyCodeMap.put(KeyCode.NUMPAD9, 81);
        keyCodeMap.put(KeyCode.NUMPAD4, 83);
        keyCodeMap.put(KeyCode.NUMPAD5, 84);
        keyCodeMap.put(KeyCode.NUMPAD6, 85);
        keyCodeMap.put(KeyCode.NUMPAD1, 87);
        keyCodeMap.put(KeyCode.NUMPAD2, 88);
        keyCodeMap.put(KeyCode.NUMPAD3, 89);
        keyCodeMap.put(KeyCode.NUMPAD0, 90);
    }

    public static Integer translate(KeyCode keyCode) {
        return keyCodeMap.get(keyCode);
    }
}
