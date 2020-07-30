

class Object {
public:

    void draw(glm::mat4 projection, glm::mat4 view, glm::mat4 model);

    int checkMouseOnMarker(glm::vec3 rayOrigin, glm::vec3 rayDirection, float& t);

    void setIntersected(int markerNum);
};