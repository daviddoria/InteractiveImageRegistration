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

#ifndef SEEDCALLBACK_H
#define SEEDCALLBACK_H

#include <vtkCommand.h>
#include <vtkSeedWidget.h>
#include <vtkSmartPointer.h>

class vtkSeedCallback : public vtkCommand
{
  public:
    static vtkSeedCallback *New()
    { 
      return new vtkSeedCallback; 
    }
    
    vtkSeedCallback() {}
    
    virtual void Execute(vtkObject*, unsigned long event, void *calldata);

    void SetWidget(vtkSmartPointer<vtkSeedWidget> widget);
    
  private:
    vtkSeedRepresentation* SeedRepresentation;
    vtkSeedWidget* SeedWidget;
};

#endif
