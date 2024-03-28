docker build -t trafficgenerator .
docker tag trafficgenerator lxlp/trafficgenerator
docker push lxlp/trafficgenerator:latest