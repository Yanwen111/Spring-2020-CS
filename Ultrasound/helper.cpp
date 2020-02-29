#include "helper.h"

int Helper::read_stl(const std::string& file_name, GLfloat * &vertices, GLfloat * &normals)
{
    std::ifstream myfile (file_name.c_str(), std::ios::in | std::ios::binary);

    char header_info[80] = "";
    char bin_n_triangles[4];
    unsigned int num_traingles = 0;

    if (myfile)
    {
        myfile.read (header_info, 80);
        std::cout <<"Header : " << header_info << std::endl;
    }

    if (myfile)
    {
        myfile.read (bin_n_triangles, 4);
        num_traingles = *((unsigned int*)bin_n_triangles) ;
        std::cout <<"Number of triangles : " << num_traingles << std::endl;
    }

    vertices = new GLfloat[num_traingles * 9];
    normals = new GLfloat[num_traingles * 9];

    int index = 0;
    int indexN = 0;
    for(int i = 0; i < num_traingles; i++)
    {
        char facet[50];
        if (myfile)
        {
            myfile.read (facet, 50);

            normals[indexN++] = *( (float*) ( ( (char*)facet)+0));
            normals[indexN++] = *( (float*) ( ( (char*)facet)+4));
            normals[indexN++] = *( (float*) ( ( (char*)facet)+8));

            normals[indexN++] = *( (float*) ( ( (char*)facet)+0));
            normals[indexN++] = *( (float*) ( ( (char*)facet)+4));
            normals[indexN++] = *( (float*) ( ( (char*)facet)+8));

            normals[indexN++] = *( (float*) ( ( (char*)facet)+0));
            normals[indexN++] = *( (float*) ( ( (char*)facet)+4));
            normals[indexN++] = *( (float*) ( ( (char*)facet)+8));

            vertices[index++] = *( (float*) ( ( (char*)facet)+12));
            vertices[index++] = *( (float*) ( ( (char*)facet)+16));
            vertices[index++] = *( (float*) ( ( (char*)facet)+20));

            vertices[index++] = *( (float*) ( ( (char*)facet)+24));
            vertices[index++] = *( (float*) ( ( (char*)facet)+28));
            vertices[index++] = *( (float*) ( ( (char*)facet)+32));

            vertices[index++] = *( (float*) ( ( (char*)facet)+36));
            vertices[index++] = *( (float*) ( ( (char*)facet)+40));
            vertices[index++] = *( (float*) ( ( (char*)facet)+44));
        }
    }
    return index;
}
