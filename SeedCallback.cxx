/*=========================================================================
 *
 *  Copyright David Doria 2011 daviddoria@gmail.com
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/

#include "SeedCallback.h"

// STL
#include <sstream>

// VTK
#include <vtkActor.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderWindow.h>
#include <vtkRendererCollection.h>
#include <vtkSeedRepresentation.h>
#include <vtkVectorText.h>

void vtkSeedCallback::Execute(vtkObject*, unsigned long event, void *calldata)
{
  if (event == vtkCommand::PlacePointEvent)
    {
    //std::cout << "Placing point..." << std::endl;
    //std::cout << "There are now " << this->SeedRepresentation->GetNumberOfSeeds() << " seeds." << std::endl;
    for(vtkIdType seedId = 0; seedId < this->SeedRepresentation->GetNumberOfSeeds(); seedId++)
      {
      double pos[3];
      this->SeedRepresentation->GetSeedDisplayPosition(seedId, pos);
      //std::cout << "Seed " << seedId << " : (" << pos[0] << " " << pos[1] << " " << pos[2] << ")" << std::endl;
      }
      
    // Create text
    vtkSmartPointer<vtkVectorText> textSource = 
      vtkSmartPointer<vtkVectorText>::New();
    std::stringstream ss;
    ss << this->SeedRepresentation->GetNumberOfSeeds()-1;
    textSource->SetText(ss.str().c_str());
    textSource->Update();
  
    // Create a mapper and actor
    vtkSmartPointer<vtkPolyDataMapper> mapper = 
      vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection(textSource->GetOutputPort());
  
    vtkSmartPointer<vtkActor> actor = 
      vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);
    actor->SetScale(10);
    
    double pos[3];
    this->SeedRepresentation->GetSeedWorldPosition(this->SeedRepresentation->GetNumberOfSeeds()-1, pos);
    actor->SetPosition(pos[0], pos[1], 0);
    //std::cout << "Text pos: " << pos[0] << " " << pos[1] << " " << 0 << std::endl;
    actor->GetProperty()->SetColor(1.0, 0.0, 0.0);

    this->SeedWidget->GetInteractor()->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->AddActor(actor);
    
    this->SeedWidget->GetInteractor()->GetRenderWindow()->Render();
    this->SeedWidget->GetInteractor()->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->Render();
    
    return;
    }
  if (event == vtkCommand::InteractionEvent)
    {
    /*
    std::cout << "Interaction..." << std::endl;
    if (calldata)
      {
      double pos[3];
      this->SeedRepresentation->GetSeedDisplayPosition(0, pos);
      std::cout << "Moved to (" << pos[0] << " " << pos[1] << " " << pos[2] << ")" << std::endl;
      }
    */
    return;
    }
}

void vtkSeedCallback::SetWidget(vtkSmartPointer<vtkSeedWidget> widget) 
{
  this->SeedWidget = widget;
  this->SeedRepresentation = vtkSeedRepresentation::SafeDownCast(this->SeedWidget->GetRepresentation());
}
