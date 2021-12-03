package pl.nieruchalski.client.domain.helpers;

import javafx.scene.input.KeyCode;

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
        keyCodeMap.put(KeyCode.BACK_QUOTE, 41); // idk
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
        keyCodeMap.put(KeyCode.BRACELEFT, 34);
        keyCodeMap.put(KeyCode.BRACERIGHT, 35);
        keyCodeMap.put(KeyCode.ENTER, 36);
        keyCodeMap.put(KeyCode.CAPS, 66);

        keyCodeMap.put(KeyCode.SHIFT, 50); // LEFT = 50 RIGHT = 62
    }

    public static Integer translate(KeyCode keyCode) {
        return keyCodeMap.get(keyCode);
    }
}
