#include <iostream>
#include <httplib.h>

void handle_request(const httplib::Request &req, httplib::Response &res) {
    // check of het een GET request is met een "data" parameter
    if (req.method == "GET" && req.has_param("data")) {
        // Extract the value of the "data" query parameter (RFID tag)
        std::string tag = req.get_param_value("data");
        
        // print tag
        std::cout << "RFID tag received: " << tag << std::endl;
        
        // response
        res.set_content("RFID tag received: " + tag, "text/plain");
    } else {
        // error handling
        res.status = 400;
        res.set_content("Bad Request", "text/plain");
    }
}

int main() {
    httplib::Server svr;

    // zorg dat inkomende http requests op de /tag endpoint worden afgehandeld
    svr.Get("/tag", handle_request);

    // start server op port 8080 op alle netwerk interfaces
    svr.listen("0.0.0.0", 8080);

    return 0;
}