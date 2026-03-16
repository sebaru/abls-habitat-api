#Creer l'image de compilation
FROM fedora:latest
LABEL maintainer="sebastien.lefevre@abls-habitat.fr"
RUN echo "Installing and Upgrading"
RUN /bin/dnf upgrade -y && dnf install git wget -y
RUN git clone -b trunk https://github.com/sebaru/abls-habitat-api /app
WORKDIR /app
RUN ./install_deps.sh
RUN ./build.sh
RUN ./install.sh
#RUN ldconfig
RUN dnf clean all
ENV ABLS_IN_A_CONTAINER=1
ENV ABLS_DB_HOSTNAME=localhost
ENV ABLS_DB_PORT=3306
ENV ABLS_HOME_URL=https://home.abls-habitat.fr
ENV ABLS_CONSOLE_URL=https://console.abls-habitat.fr
ENV ABLS_IDP_URL=https://idp.abls-habitat.fr
ENV ABLS_IDP_REALM=Abls-Habitat
ENV ABLS_API_LOCAL_PORT=5562
ENV ABLS_STATIC_DATA_URL=https://static.abls-habitat.fr
ENV ABLS_MQTT_HOSTNAME=127.0.0.1
ENV ABLS_MQTT_PORT=1883
ENV ABLS_MQTT_PASSWORD=changeme
ENV ABLS_MQTT_QOS=1
ENV ABLS_LOG_LEVEL=3
EXPOSE 5562
CMD /usr/local/bin/abls-habitat-api