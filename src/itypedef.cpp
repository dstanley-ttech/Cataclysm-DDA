#include "itype.h"
#include "ammo.h"
#include "game.h"
#include "monstergenerator.h"
#include "item_factory.h"
#include <fstream>
#include <stdexcept>

// Members of iuse struct, which is slowly morphing into a class.
bool itype::has_use() const
{
    return !use_methods.empty();
}

bool itype::can_use( const std::string &iuse_name ) const
{
    return get_use( iuse_name ) != nullptr;
}

const use_function *itype::get_use( const std::string &iuse_name ) const
{
    if( !has_use() ) {
        return nullptr;
    }

    const use_function *func = item_controller->get_iuse( iuse_name );
    if( func != nullptr ) {
        if( std::find( use_methods.cbegin(), use_methods.cend(),
                         *func ) != use_methods.cend() ) {
            return func;
        }
    }

    for( const auto &method : use_methods ) {
        const auto ptr = method.get_actor_ptr();
        if( ptr != nullptr && ptr->type == iuse_name ) {
            return &method;
        }
    }

    return nullptr;
}

long itype::tick( player *p, item *it, point pos ) const
{
    // Note: can go higher than current charge count
    // Maybe should move charge decrementing here?
    int charges_to_use = 0;
    for( auto &method : use_methods ) {
        int val = method.call( p, it, true, pos );
        if( charges_to_use < 0 || val < 0 ) {
            charges_to_use = -1;
        } else {
            charges_to_use += val;
        }
    }

    return charges_to_use;
}

long itype::invoke( player *p, item *it, point pos ) const
{
    if( !has_use() ) {
        return 0;
    }

    return use_methods.front().call( p, it, false, pos );
}

long itype::invoke( player *p, item *it, point pos, const std::string &iuse_name ) const
{
    const use_function *use = get_use( iuse_name );
    if( use == nullptr ) {
        debugmsg( "Tried to invoke %s on a %s, which doesn't have this use_function",
                  iuse_name.c_str(), nname(1).c_str() );
        return 0;
    }

    return use->call( p, it, false, pos );
}

itype *newSoftwareIType( const itype_id &id, const std::string &name, const std::string &name_plural,
               unsigned int price, software_type swtype, int /*power*/, const std::string &description )
{
    static const std::vector<std::string> no_materials = { "null" };
    itype *t = new itype( id, price, name, name_plural, description, ' ', c_white, no_materials,
                          SOLID, 0, 0, 0, 0, 0 );
    t->software.reset( new islot_software() );
    t->software->swtype = swtype;
    t->spawn.reset( new islot_spawn() );
    t->spawn->default_container = "usb_drive";
    return t;
}

void Item_factory::init_old()
{
    auto &itypes = m_templates; // So I don't have to change that in the code below

    std::vector<std::string> no_materials = { "null" };

    // First, the null object.  NOT REALLY AN OBJECT AT ALL.  More of a concept.
    add_item_type(
        new itype("null", 0, "none", "none", "", '#', c_white, no_materials, PNULL,
                  0, 0, 0, 0, 0) );
    itypes["null"]->item_tags.insert( "PSEUDO" );

    add_item_type( newSoftwareIType( "software_useless", "misc software", "none", 300, SW_USELESS, 0,
    _( "A miscellaneous piece of hobby software. Probably useless." ) ) );

    add_item_type( newSoftwareIType( "software_hacking", "hackPRO", "none", 800, SW_HACKING, 2,
    _( "A piece of hacking software." ) ) );

    add_item_type( newSoftwareIType( "software_medical", "MediSoft", "none", 600, SW_MEDICAL, 2,
    _( "A piece of medical software." ) ) );

    add_item_type( newSoftwareIType( "software_math", "MatheMAX", "none", 500, SW_SCIENCE, 3,
    _( "A piece of mathematical software." ) ) );

    add_item_type( newSoftwareIType( "software_blood_data", "infection data", "none", 200, SW_DATA, 5,
    _( "Medical data on zombie blood." ) ) );
}

std::string const& ammo_name(std::string const &t)
{
    return ammunition_type::find_ammunition_type(t).name();
}

itype_id const& default_ammo(std::string const &t)
{
    return ammunition_type::find_ammunition_type(t).default_ammotype();
}
