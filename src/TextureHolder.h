#ifndef textureholder_h
#define textureholder_h

#include <SFML/Graphics.hpp>
#include <map>

using namespace sf;
using namespace std;

class TextureHolder
{
public:
	TextureHolder();
	static Texture& GetTexture(string const& filename);
private:
	// a map holding pairs of string + texture
	map<string, Texture> m_Textures;

	static TextureHolder* m_s_Instance;
};
#endif 
