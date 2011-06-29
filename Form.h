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

#ifndef FORM_H
#define FORM_H

#include "ui_Form.h"

// VTK
#include <vtkSmartPointer.h>
#include <vtkSeedWidget.h>
#include <vtkPointHandleRepresentation2D.h>

// ITK
#include "itkImage.h"

// Qt
#include <QMainWindow>

// Custom
#include "Types.h"
#include "SeedCallback.h"

// Forward declarations
class vtkRenderer;
class vtkBorderWidget;
class BorderCallbackClass;
class vtkImageData;
class vtkImageActor;
class vtkActor;

class Form : public QMainWindow, public Ui::Form
{
  Q_OBJECT
public:

  // Constructor/Destructor
  Form();
  ~Form() {};

public slots:
  void on_actionOpenMovingImage_activated();
  void on_actionOpenFixedImage_activated();
  void on_actionSave_activated();

protected:

  vtkSmartPointer<vtkRenderer> LeftRenderer;
  vtkSmartPointer<vtkRenderer> RightRenderer;
  
  // Fixed image
  FloatVectorImageType::Pointer FixedImage;
  vtkSmartPointer<vtkImageActor> FixedImageActor;
  vtkSmartPointer<vtkImageData> FixedImageData;
  
  // Moving image
  FloatVectorImageType::Pointer MovingImage;
  vtkSmartPointer<vtkImageActor> MovingImageActor;
  vtkSmartPointer<vtkImageData> MovingImageData;
  
  // Transformed image
  FloatVectorImageType::Pointer TransformedImage;
  vtkSmartPointer<vtkImageActor> TransformedImageActor;
  vtkSmartPointer<vtkImageData> TransformedImageData;  
  
  vtkSmartPointer<vtkSeedWidget> FixedSeedWidget;
  vtkSmartPointer<vtkSeedWidget> MovingSeedWidget;

  vtkSmartPointer<vtkSeedCallback> FixedSeedCallback;
  vtkSmartPointer<vtkSeedCallback> MovingSeedCallback;

  vtkSmartPointer<vtkPointHandleRepresentation2D> HandleRepresentation;
  vtkSmartPointer<vtkSeedRepresentation> SeedRepresentation;
};

#endif // Form_H
