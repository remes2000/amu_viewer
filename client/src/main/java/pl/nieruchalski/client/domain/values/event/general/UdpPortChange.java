package pl.nieruchalski.client.domain.values.event.general;

public class UdpPortChange extends GeneralInformation {
    private Integer port;
    public UdpPortChange(Integer port) {
        this.port = port;
    }

    public Integer getPort() {
        return port;
    }
}
