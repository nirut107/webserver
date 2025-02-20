#include "Server.hpp"

int main(int argc, char **argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <config_file>" << std::endl;
        return 1;
    }

    std::vector<Server> servers = parseConfig(argv[1]);

    // Iterate through servers using iterators (C++98)
    for (std::vector<Server>::const_iterator it = servers.begin(); it != servers.end(); ++it) {
        std::cout << "Server listening on: " << it->getPort() << std::endl;
		std::cout << "Server name: " << it->getServerName() << std::endl;
        std::cout << "Root directory: " << it->getRoot() << std::endl;

        // Iterate through locations using iterators (C++98)
        std::map<std::string, t_location> locations = it->getLocations();
        for (std::map<std::string, t_location>::const_iterator loc_it = locations.begin(); loc_it != locations.end(); ++loc_it) {
            std::cout << "  Location: " << loc_it->first << ", Index: " << loc_it->second.index << std::endl;
        }
    }

    return 0;
}
