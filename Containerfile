#Creer l'image de compilation
FROM fedora:latest AS builder
LABEL maintainer="sebastien.lefevre@abls-habitat.fr"
RUN echo "Installing and Upgrading"
RUN /bin/dnf upgrade -y && dnf install git wget -y && dnf clean all
WORKDIR /app
COPY ./ ./
RUN ./INSTALL.sh
RUN ./autogen.sh
RUN make install
#RUN ldconfig
ENV ABLS_IN_A_CONTAINER 1
#CMD /usr/local/bin/abls-habitat-api

# Créer une image légère pour l'exécution
FROM fedora:latest
LABEL maintainer="sebastien.lefevre@abls-habitat.fr"
RUN echo "Upgrading"
RUN /bin/dnf upgrade -y && dnf clean all
EXPOSE 5562
WORKDIR /app
# Copier uniquement l'exécutable final depuis l'étape de build
COPY --from=builder /usr/local/bin/abls-habitat-api .
# Lancer l'application
CMD ["/app/abls-habitat-api"]
