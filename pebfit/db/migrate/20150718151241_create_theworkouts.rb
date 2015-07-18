class CreateTheworkouts < ActiveRecord::Migration
  def change
    create_table :theworkouts do |t|
      t.string :name
      t.string :totaltime
      t.date :lastattempt

      t.timestamps null: false
    end
  end
end
