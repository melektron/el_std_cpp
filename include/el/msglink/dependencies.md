# msglink dependencies

The C++ implementation of msglink currently uses Niels Lohmann's JSON library as well as WebSocket++.
Both of those are bundled or provided as submodules.

WebSocket++ needs ASIO for network communication. It needs to be installed, ideally in the standalone version without boost:

```bash
# On debian or derivatives:
sudo apt install libasio-dev
```