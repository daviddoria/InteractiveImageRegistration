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

#include "ui_Form.h"
#include "Form.h"

// ITK
#include "itkCastImageFilter.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkRegionOfInterestImageFilter.h"

// Qt
#include <QFileDialog>
#include <QIcon>

// VTK
#include <vtkActor.h>
#include <vtkCommand.h>
#include <vtkDataSetSurfaceFilter.h>
#include <vtkImageActor.h>
#include <vtkImageData.h>
#include <vtkInteractorStyleImage.h>
#include <vtkMath.h>
#include <vtkPointData.h>
#include <vtkProperty2D.h>
#include <vtkPolyDataMapper.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSeedRepresentation.h>
#include <vtkSeedWidget.h>
#include <vtkSmartPointer.h>
#include <vtkVertexGlyphFilter.h>

// Custom
#include "Helpers.h"
#include "Types.h"

// Constructor
Form::Form()
{
  this->setupUi(this);

  this->LeftRenderer = vtkSmartPointer<vtkRenderer>::New();
  this->RightRenderer = vtkSmartPointer<vtkRenderer>::New();

  this->qvtkWidgetLeft->GetRenderWindow()->AddRenderer(this->LeftRenderer);
  this->qvtkWidgetRight->GetRenderWindow()->AddRenderer(this->RightRenderer);

  this->MovingImageActor = vtkSmartPointer<vtkImageActor>::New();
  this->MovingImageData = vtkSmartPointer<vtkImageData>::New();
  
  this->FixedImageActor = vtkSmartPointer<vtkImageActor>::New();
  this->FixedImageData = vtkSmartPointer<vtkImageData>::New();
  
  this->TransformedImageActor = vtkSmartPointer<vtkImageActor>::New();
  this->TransformedImageData = vtkSmartPointer<vtkImageData>::New();
  
  // Setup toolbar
  QIcon openIcon = QIcon::fromTheme("document-open");
  actionOpenFixedImage->setIcon(openIcon);
  this->toolBar->addAction(actionOpenFixedImage);
  
  actionOpenMovingImage->setIcon(openIcon);
  this->toolBar->addAction(actionOpenMovingImage);
  
  QIcon saveIcon = QIcon::fromTheme("document-save");
  actionSave->setIcon(saveIcon);
  this->toolBar->addAction(actionSave);
  
  this->HandleRepresentation = vtkSmartPointer<vtkPointHandleRepresentation2D>::New();
  this->HandleRepresentation->GetProperty()->SetColor(1,0,0);

  this->SeedRepresentation = vtkSmartPointer<vtkSeedRepresentation>::New();
  this->SeedRepresentation->SetHandleRepresentation(this->HandleRepresentation);
};

void Form::on_actionOpenMovingImage_activated()
{
   // Get a filename to open
  QString fileName = QFileDialog::getOpenFileName(this, "Open File", ".", "Image Files (*.png *.mhd *.tif)");

  std::cout << "Got filename: " << fileName.toStdString() << std::endl;
  if(fileName.toStdString().empty())
    {
    std::cout << "Filename was empty." << std::endl;
    return;
    }

  typedef itk::ImageFileReader<FloatVectorImageType> ReaderType;
  ReaderType::Pointer reader = ReaderType::New();
  reader->SetFileName(fileName.toStdString());
  reader->Update();
  
  this->MovingImage = reader->GetOutput();

  if(this->chkRGB->isChecked())
    {
    Helpers::ITKImagetoVTKRGBImage(this->MovingImage, this->MovingImageData);
    }
  else
    {
    Helpers::ITKImagetoVTKMagnitudeImage(this->MovingImage, this->MovingImageData);
    }
  
  this->MovingImageActor->SetInput(this->MovingImageData);

  // Add Actor to renderer
  this->RightRenderer->AddActor(this->MovingImageActor);
  this->RightRenderer->ResetCamera();

  vtkSmartPointer<vtkInteractorStyleImage> interactorStyle =
      vtkSmartPointer<vtkInteractorStyleImage>::New();
  this->qvtkWidgetRight->GetRenderWindow()->GetInteractor()->SetInteractorStyle(interactorStyle);

  this->RightRenderer->ResetCamera();

  
  // Seed widget
  this->MovingSeedWidget = vtkSmartPointer<vtkSeedWidget>::New();
  this->MovingSeedWidget->SetInteractor(this->qvtkWidgetRight->GetRenderWindow()->GetInteractor());
  this->MovingSeedWidget->SetRepresentation(this->SeedRepresentation);
  
  this->MovingSeedCallback = vtkSmartPointer<vtkSeedCallback>::New();
  this->MovingSeedCallback->SetWidget(this->MovingSeedWidget);
  
  this->MovingSeedWidget->AddObserver(vtkCommand::PlacePointEvent,this->MovingSeedCallback);
  this->MovingSeedWidget->AddObserver(vtkCommand::InteractionEvent,this->MovingSeedCallback);
  this->MovingSeedWidget->On();
}

void Form::on_actionOpenFixedImage_activated()
{
  // Get a filename to open
  QString fileName = QFileDialog::getOpenFileName(this, "Open File", ".", "Image Files (*.png *.mhd *.tif)");

  std::cout << "Got filename: " << fileName.toStdString() << std::endl;
  if(fileName.toStdString().empty())
    {
    std::cout << "Filename was empty." << std::endl;
    return;
    }

  typedef itk::ImageFileReader<FloatVectorImageType> ReaderType;
  ReaderType::Pointer reader = ReaderType::New();
  reader->SetFileName(fileName.toStdString());
  reader->Update();
  
  this->FixedImage = reader->GetOutput();

  if(this->chkRGB->isChecked())
    {
    Helpers::ITKImagetoVTKRGBImage(this->FixedImage, this->FixedImageData);
    }
  else
    {
    Helpers::ITKImagetoVTKMagnitudeImage(this->FixedImage, this->FixedImageData);
    }
  
  this->FixedImageActor->SetInput(this->FixedImageData);

  // Add Actor to renderer
  this->LeftRenderer->AddActor(this->FixedImageActor);
  this->LeftRenderer->ResetCamera();

  vtkSmartPointer<vtkInteractorStyleImage> interactorStyle =
      vtkSmartPointer<vtkInteractorStyleImage>::New();
  this->qvtkWidgetLeft->GetRenderWindow()->GetInteractor()->SetInteractorStyle(interactorStyle);

  this->LeftRenderer->ResetCamera();

  // Seed widget
  this->FixedSeedWidget = vtkSmartPointer<vtkSeedWidget>::New();
  this->FixedSeedWidget->SetInteractor(this->qvtkWidgetLeft->GetRenderWindow()->GetInteractor());
  this->FixedSeedWidget->SetRepresentation(this->SeedRepresentation);
  
  this->FixedSeedCallback = vtkSmartPointer<vtkSeedCallback>::New();
  this->FixedSeedCallback->SetWidget(this->FixedSeedWidget);
  
  this->FixedSeedWidget->AddObserver(vtkCommand::PlacePointEvent,this->FixedSeedCallback);
  this->FixedSeedWidget->AddObserver(vtkCommand::InteractionEvent,this->FixedSeedCallback);
  this->FixedSeedWidget->On();
}

void Form::on_actionSave_activated()
{
  if(this->chkRGB->isChecked())
    {
    QString fileName = QFileDialog::getSaveFileName(this, "Save File", ".", "Image Files (*.png)");
    std::cout << "Got filename: " << fileName.toStdString() << std::endl;
    if(fileName.toStdString().empty())
      {
      std::cout << "Filename was empty." << std::endl;
      return;
      }
    typedef itk::CastImageFilter< FloatVectorImageType, UnsignedCharVectorImageType > CastFilterType;
    CastFilterType::Pointer castFilter = CastFilterType::New();
    castFilter->SetInput(this->TransformedImage);
    castFilter->Update();
    
    typedef  itk::ImageFileWriter< UnsignedCharVectorImageType  > WriterType;
    WriterType::Pointer writer = WriterType::New();
    writer->SetFileName(fileName.toStdString());
    writer->SetInput(castFilter->GetOutput());
    writer->Update();
    }
  else
    {
    QString fileName = QFileDialog::getSaveFileName(this, "Save File", ".", "Image Files (*.mhd)");
    std::cout << "Got filename: " << fileName.toStdString() << std::endl;
    if(fileName.toStdString().empty())
      {
      std::cout << "Filename was empty." << std::endl;
      return;
      }
    typedef  itk::ImageFileWriter< FloatVectorImageType  > WriterType;
    WriterType::Pointer writer = WriterType::New();
    writer->SetFileName(fileName.toStdString());
    writer->SetInput(this->TransformedImage);
    writer->Update();
    }

}
