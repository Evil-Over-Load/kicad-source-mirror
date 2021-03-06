/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2020 Joshua Redstone redstone at gmail.com
 * Copyright (C) 1992-2020 KiCad Developers, see AUTHORS.txt for contributors.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you may find one here:
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 * or you may search the http://www.gnu.org website for the version 2 license,
 * or you may write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */
#include <bitmaps.h>
#include <class_board.h>
#include <class_board_item.h>
#include <class_pcb_group.h>
#include <confirm.h>
#include <msgpanel.h>
#include <view/view.h>

PCB_GROUP::PCB_GROUP( BOARD_ITEM* aParent ) :
        BOARD_ITEM( aParent, PCB_GROUP_T )
{
}


bool PCB_GROUP::AddItem( BOARD_ITEM* aItem )
{
    // Items can only be in one group at a time
    if( aItem->GetParentGroup() )
        aItem->GetParentGroup()->RemoveItem( aItem );

    m_items.insert( aItem );
    aItem->SetParentGroup( this );
    return true;
}


bool PCB_GROUP::RemoveItem( BOARD_ITEM* aItem )
{
    // Only clear the item's group field if it was inside this group
    if( m_items.erase( aItem ) == 1 )
    {
        aItem->SetParentGroup( nullptr );
        return true;
    }

    return false;
}


void PCB_GROUP::RemoveAll()
{
    for( BOARD_ITEM* item : m_items )
        item->SetParentGroup( nullptr );

    m_items.clear();
}


PCB_GROUP* PCB_GROUP::TopLevelGroup( BOARD_ITEM* item, PCB_GROUP* scope )
{
    PCB_GROUP* candidate = item->GetParentGroup();

    if( !candidate && item->GetParent() && item->GetParent()->Type() == PCB_MODULE_T )
        candidate = item->GetParent()->GetParentGroup();

    while( candidate && candidate->GetParentGroup() && candidate->GetParentGroup() != scope )
        candidate = candidate->GetParentGroup();

    return candidate == scope ? nullptr : candidate;
}


bool PCB_GROUP::WithinScope( BOARD_ITEM* item, PCB_GROUP* scope )
{
    if( item->GetParent() && item->GetParent()->Type() == PCB_MODULE_T )
        item = item->GetParent();

    for( PCB_GROUP* parent = item->GetParentGroup(); parent; parent = parent->GetParentGroup() )
    {
        if( parent == scope )
            return true;
    }

    return false;
}


wxPoint PCB_GROUP::GetPosition() const
{
    return GetBoundingBox().Centre();
}


void PCB_GROUP::SetPosition( const wxPoint& aNewpos )
{
    wxPoint delta = aNewpos - GetPosition();

    Move( delta );
}


EDA_ITEM* PCB_GROUP::Clone() const
{
    // Use copy constructor to get the same uuid and other fields
    PCB_GROUP* newGroup = new PCB_GROUP( *this );
    return newGroup;
}


PCB_GROUP* PCB_GROUP::DeepClone() const
{
    // Use copy constructor to get the same uuid and other fields
    PCB_GROUP* newGroup = new PCB_GROUP( *this );
    newGroup->m_items.clear();

    for( BOARD_ITEM* member : m_items )
    {
        if( member->Type() == PCB_GROUP_T )
            newGroup->AddItem( static_cast<PCB_GROUP*>( member )->DeepClone() );
        else
            newGroup->AddItem( static_cast<BOARD_ITEM*>( member->Clone() ) );
    }

    return newGroup;
}


PCB_GROUP* PCB_GROUP::DeepDuplicate() const
{
    PCB_GROUP* newGroup = static_cast<PCB_GROUP*>( this->Duplicate() );
    newGroup->m_items.clear();

    for( BOARD_ITEM* member : m_items )
    {
        if( member->Type() == PCB_GROUP_T )
            newGroup->AddItem( static_cast<PCB_GROUP*>( member )->DeepDuplicate() );
        else
            newGroup->AddItem( static_cast<BOARD_ITEM*>( member->Duplicate() ) );
    }

    return newGroup;
}


void PCB_GROUP::SwapData( BOARD_ITEM* aImage )
{
    assert( aImage->Type() == PCB_GROUP_T );

    std::swap( *( (PCB_GROUP*) this ), *( (PCB_GROUP*) aImage ) );
}


bool PCB_GROUP::HitTest( const wxPoint& aPosition, int aAccuracy ) const
{
    // Groups are selected by promoting a selection of one of their children
    return false;
}


bool PCB_GROUP::HitTest( const EDA_RECT& aRect, bool aContained, int aAccuracy ) const
{
    // Groups are selected by promoting a selection of one of their children
    return false;
}


const EDA_RECT PCB_GROUP::GetBoundingBox() const
{
    EDA_RECT area;

    for( BOARD_ITEM* item : m_items )
        area.Merge( item->GetBoundingBox() );

    area.Inflate( Millimeter2iu( 0.25 ) ); // Give a min size to the area

    return area;
}


SEARCH_RESULT PCB_GROUP::Visit( INSPECTOR aInspector, void* aTestData, const KICAD_T aScanTypes[] )
{
    for( const KICAD_T* stype = aScanTypes; *stype != EOT; ++stype )
    {
        // If caller wants to inspect my type
        if( *stype == Type() )
        {
            if( SEARCH_RESULT::QUIT == aInspector( this, aTestData ) )
                return SEARCH_RESULT::QUIT;
        }
    }

    return SEARCH_RESULT::CONTINUE;
}


LSET PCB_GROUP::GetLayerSet() const
{
    LSET aSet;

    for( BOARD_ITEM* item : m_items )
        aSet |= item->GetLayerSet();

    return aSet;
}


bool PCB_GROUP::IsOnLayer( PCB_LAYER_ID aLayer ) const
{
    // A group is on a layer if any item is on the layer
    for( BOARD_ITEM* item : m_items )
    {
        if( item->IsOnLayer( aLayer ) )
            return true;
    }

    return false;
}


void PCB_GROUP::ViewGetLayers( int aLayers[], int& aCount ) const
{
    // What layer to put bounding box on?  change in class_pcb_group.cpp
    std::unordered_set<int> layers = { LAYER_ANCHOR }; // for bounding box

    for( BOARD_ITEM* item : m_items )
    {
        int member_layers[KIGFX::VIEW::VIEW_MAX_LAYERS], member_layers_count;
        item->ViewGetLayers( member_layers, member_layers_count );

        for( int i = 0; i < member_layers_count; i++ )
            layers.insert( member_layers[i] );
    }

    aCount = layers.size();
    int i  = 0;

    for( int layer : layers )
        aLayers[i++] = layer;
}


double PCB_GROUP::ViewGetLOD( int aLayer, KIGFX::VIEW* aView ) const
{
    if( aView->IsLayerVisible( LAYER_ANCHOR ) )
        return 0.0;

    return std::numeric_limits<double>::max();
}


void PCB_GROUP::Move( const wxPoint& aMoveVector )
{
    for( BOARD_ITEM* member : m_items )
        member->Move( aMoveVector );
}


void PCB_GROUP::Rotate( const wxPoint& aRotCentre, double aAngle )
{
    for( BOARD_ITEM* item : m_items )
        item->Rotate( aRotCentre, aAngle );
}


void PCB_GROUP::Flip( const wxPoint& aCentre, bool aFlipLeftRight )
{
    for( BOARD_ITEM* item : m_items )
        item->Flip( aCentre, aFlipLeftRight );
}


wxString PCB_GROUP::GetSelectMenuText( EDA_UNITS aUnits ) const
{
    if( m_name.empty() )
    {
        return wxString::Format( _( "Anonymous Group, %zu members" ),
                                 m_items.size() );
    }

    return wxString::Format( _( "Group \"%s\", %zu members" ),
                             m_name,
                             m_items.size() );
}


BITMAP_DEF PCB_GROUP::GetMenuImage() const
{
    return module_xpm;
}


void PCB_GROUP::GetMsgPanelInfo( EDA_DRAW_FRAME* aFrame, std::vector<MSG_PANEL_ITEM>& aList )
{
    aList.emplace_back( _( "Group" ), m_name.empty() ? _( "Anonymous" ) :
                        wxString::Format( "\"%s\"", m_name ), DARKCYAN );
    aList.emplace_back( _( "Members" ), wxString::Format( "%zu", m_items.size() ), BROWN );
}


void PCB_GROUP::RunOnChildren( const std::function<void( BOARD_ITEM* )>& aFunction )
{
    try
    {
        for( BOARD_ITEM* item : m_items )
            aFunction( item );
    }
    catch( std::bad_function_call& )
    {
        wxFAIL_MSG( wxT( "Error calling function in PCB_GROUP::RunOnChildren" ) );
    }
}


void PCB_GROUP::RunOnDescendants( const std::function<void( BOARD_ITEM* )>& aFunction )
{
    try
    {
        for( BOARD_ITEM* item : m_items )
        {
            aFunction( item );

            if( item->Type() == PCB_GROUP_T )
                static_cast<PCB_GROUP*>( item )->RunOnDescendants( aFunction );
        }
    }
    catch( std::bad_function_call& )
    {
        wxFAIL_MSG( wxT( "Error calling function in PCB_GROUP::RunOnDescendants" ) );
    }
}
