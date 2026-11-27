// Copyright Epic Games, Inc. All Rights Reserved.

#include "CombatUsePromptWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/TextBlock.h"
#include "Styling/CoreStyle.h"

void UCombatUsePromptWidget::UpdatePromptText_Implementation(const FText& KeyText, const FText& ActionText)
{
        SetPromptText(KeyText, ActionText);
}

void UCombatUsePromptWidget::NativeOnInitialized()
{
        Super::NativeOnInitialized();

        // Blueprint subclasses come with their own designed tree; only build the
        // fallback for pure C++ widgets, before the root widget is taken into Slate
        if (!WidgetTree->RootWidget)
        {
                BuildDefaultLayout();
        }
}

void UCombatUsePromptWidget::BuildDefaultLayout()
{
        // rounded dark panel the labels sit on
        UBorder* Panel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("PromptPanel"));
        Panel->SetPadding(FMargin(14.0f, 8.0f, 14.0f, 8.0f));

        FSlateBrush PanelBrush;
        PanelBrush.DrawAs = ESlateBrushDrawType::RoundedBox;
        PanelBrush.TintColor = FLinearColor(0.02f, 0.02f, 0.03f, 0.65f);
        PanelBrush.OutlineSettings.Color = FLinearColor(1.0f, 1.0f, 1.0f, 0.35f);
        PanelBrush.OutlineSettings.Width = 1.0f;
        PanelBrush.OutlineSettings.RoundingType = ESlateBrushRoundingType::FixedRadius;
        Panel->Background = PanelBrush;

        WidgetTree->RootWidget = Panel;

        UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("PromptRow"));
        Panel->SetContent(Row);

        // "[E] Open": highlighted key label + plain action label
        KeyTextBlock = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("KeyLabel"));
        KeyTextBlock->SetFont(FCoreStyle::GetDefaultFontStyle("Bold", 16));
        KeyTextBlock->SetColorAndOpacity(FLinearColor(1.0f, 0.82f, 0.35f, 1.0f));
        KeyTextBlock->SetText(FText::FromString(TEXT("[E]")));

        ActionTextBlock = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ActionLabel"));
        ActionTextBlock->SetFont(FCoreStyle::GetDefaultFontStyle("Regular", 16));
        ActionTextBlock->SetColorAndOpacity(FLinearColor::White);
        ActionTextBlock->SetText(NSLOCTEXT("CombatUsePrompt", "DefaultActionText", "Use"));

        if (UHorizontalBoxSlot* KeySlot = Row->AddChildToHorizontalBox(KeyTextBlock))
        {
                KeySlot->SetPadding(FMargin(0.0f, 0.0f, 8.0f, 0.0f));
                KeySlot->SetVerticalAlignment(VAlign_Center);
        }

        if (UHorizontalBoxSlot* ActionSlot = Row->AddChildToHorizontalBox(ActionTextBlock))
        {
                ActionSlot->SetVerticalAlignment(VAlign_Center);
        }
}

void UCombatUsePromptWidget::SetPromptText(const FText& KeyText, const FText& ActionText)
{
        if (KeyTextBlock)
        {
                KeyTextBlock->SetText(FText::Format(NSLOCTEXT("CombatUsePrompt", "KeyBracketFormat", "[{0}]"), KeyText));
        }

        if (ActionTextBlock)
        {
                ActionTextBlock->SetText(ActionText);
        }

        // let Blueprint subclasses with their own layout react to the change
        BP_OnPromptTextChanged(KeyText, ActionText);
}
