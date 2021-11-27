package pl.nieruchalski.client.domain.values;

import org.apache.commons.validator.routines.InetAddressValidator;
import pl.nieruchalski.client.domain.exception.InvalidConnectionRequestDataException;

public class ConnectionRequest {
    private String ip;
    private Integer port;
    private Integer accessCode;

    public ConnectionRequest(String ip, String port, String accessCode) {
        this.setIp(ip);
        this.setPort(port);
        this.setAccessCode(accessCode);
    }

    private void setIp(String ip) {
        if(!InetAddressValidator.getInstance().isValidInet4Address(ip)) {
            throw new InvalidConnectionRequestDataException("Invalid ip address");
        }
        this.ip = ip;
    }

    private void setPort(String port) {
        try {
            Integer asNumber = Integer.parseInt(port);
            if(asNumber < 0 || asNumber > 65535) {
                throw new InvalidConnectionRequestDataException("Invalid port");
            }
            this.port = asNumber;
        } catch (NumberFormatException e) {
            throw new InvalidConnectionRequestDataException("Invalid port");
        }
    }

    private void setAccessCode(String accessCode) {
        try {
            Integer asNumber = Integer.parseInt(accessCode);
            if(asNumber < 100 || asNumber > 999) {
                throw new InvalidConnectionRequestDataException("Invalid access code");
            }
            this.accessCode = asNumber;
        } catch (NumberFormatException e) {
            throw new InvalidConnectionRequestDataException("Invalid access code");
        }
    }

    public String getIp() {
        return ip;
    }

    public Integer getPort() {
        return port;
    }

    public Integer getAccessCode() {
        return accessCode;
    }
}
